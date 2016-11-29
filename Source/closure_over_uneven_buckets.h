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
  std::vector<int>& bucket_id);

// a version of the above function with some buckets partially pre-filled
void ClosureOverUnevenBuckets(const std::vector<float>& object,
  const std::vector<float>& bucket,
  std::vector<int>& bucket_id,
  std::vector<float> pre_fill);

#endif
