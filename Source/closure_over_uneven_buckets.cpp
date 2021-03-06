/*
  ==============================================================================

    closure_over_uneven_buckets.cpp
    Created: 18 Nov 2016 10:37:35am
    Author:  falindrith

  ==============================================================================
*/

#include "closure_over_uneven_buckets.h"

void ClosureOverUnevenBuckets(const std::vector<float>& object,
  const std::vector<float>& bucket,
  std::vector<int>& bucket_id)
{
  std::vector<float> pre_fill;
  pre_fill.resize(bucket_id.size(), 0);

  ClosureOverUnevenBuckets(object, bucket, bucket_id, pre_fill);
  
}

void ClosureOverUnevenBuckets(const std::vector<float>& object, const std::vector<float>& bucket,
  std::vector<int>& bucket_id, std::vector<float> pre_fill)
{
  if (bucket_id.size() != object.size())
  {
    bucket_id.resize(object.size(), -1);
  }

  if (pre_fill.size() != bucket_id.size()) {
    pre_fill.resize(bucket_id.size(), 0);
  }

  float sum_values = 0.0f;
  std::vector<int> closure_drip_order(object.size());
  for (int i = 0; i < object.size(); ++i)
  {
    sum_values += object[i];
    closure_drip_order[i] = i;
  }

  // don't forget to add pre-filled to total capacity
  for (float v : pre_fill) {
    sum_values += v;
  }

  // we have some prefilled buckets, so remove that from the remaining capactiy
  std::vector<float> remaining_bucket_capacity(bucket.size());
  for (int i = 0; i < bucket.size(); ++i)
  {
    remaining_bucket_capacity[i] = sum_values * bucket[i] - pre_fill[i];
  }

  // Make a random dripping closure
  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(closure_drip_order.begin(), closure_drip_order.end(), gen);

  // Now drip into buckets
  int current_bucket = 0;
  for (int i = 0; i < object.size(); ++i)
  {
    if (object[closure_drip_order[i]] < remaining_bucket_capacity[current_bucket])
    {
      // cool, it fits
      bucket_id[closure_drip_order[i]] = current_bucket;
      current_bucket = (current_bucket + 1) % bucket.size();
    }
    else
    {
      // find max remaining capacity and put it there
      auto it = std::max_element(std::begin(remaining_bucket_capacity),
        std::end(remaining_bucket_capacity));
      bucket_id[closure_drip_order[i]] = (int)(it - remaining_bucket_capacity.begin());
    }
    remaining_bucket_capacity[bucket_id[closure_drip_order[i]]] =
      remaining_bucket_capacity[bucket_id[closure_drip_order[i]]] - object[closure_drip_order[i]];
  }
}
