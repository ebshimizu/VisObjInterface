#pragma once
#ifndef _CLOSURE_OVER_UNEVEN_BUCKETS_H_
#define _CLOSURE_OVER_UNEVEN_BUCKETS_H_

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>

// This algorithm solves the following problem using a closure dripping over uneven
// buckets approach.
// Problem: K discrete objects with positive values and N buckets with uneven
//          relative capacities are given. We need to randomly distribute the objects
//          into the buckets such that they loosely meet the relative capacities
//          without hurting the randomness of the object order significantly.
//
// yumer, Nov-17-2016
//
//
// inputs:
// object = vector of size K, with values of stored for each object.
// bucket = vector of size N, with normalized relative capacity of buckets.
//          (sum(bucket)=1.0)
//
// output:
// bucket_id = vector of size K, with bucket ids assigned to each object.
//             (bucket_id[i] \in [0,N-1])
//
void ClosureOverUnevenBuckets(const std::vector<float>& object,
                              const std::vector<float>& bucket,
                              std::vector<int>& bucket_id)
{
  if (bucket_id.size() != object.size())
  {
    bucket_id.resize(object.size(), -1);
  }

  float sum_values = 0.0f;
  std::vector<int> closure_drip_order(object.size());
  for (int i = 0; i < object.size(); ++i)
  {
    sum_values += object[i];
    closure_drip_order[i] = i;
  }

  std::vector<float> remaining_bucket_capacity(bucket.size());
  for (int i = 0; i < bucket.size(); ++i)
  {
    remaining_bucket_capacity[i] = sum_values * bucket[i];
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
  }
}

#endif
