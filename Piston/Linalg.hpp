#ifndef LINALG_HPP
#define LINALG_HPP
#include <math.h>

typedef struct {
  float x;
  float y;
} vector2_T;

inline float RectFromPoints_m(vector2_T a, vector2_T b) {
  return (a.y - b.y) / (a.x - b.x);
}

inline float dotProduct(vector2_T arr1, vector2_T arr2) {
  return (arr1.x * arr2.x) + (arr1.y * arr2.y);
}

inline vector2_T subVec(vector2_T arr1, vector2_T arr2) {
  return (vector2_T){.x = arr1.x - arr2.x, .y = arr1.y - arr2.y};
}

inline vector2_T addVec(vector2_T arr1, vector2_T arr2) {
  return (vector2_T){.x = (arr1.x + arr2.x), .y = (arr1.y + arr2.y)};
}

inline float squareVector(vector2_T vector) {
  return powf(vector.x, 2) + powf(vector.y, 2);
}

inline vector2_T scalarProductVec(vector2_T arr1, float scalar) {
  return (vector2_T){.x = arr1.x * scalar, .y = arr1.y * scalar};
}

inline float getDistance(vector2_T pos1, vector2_T pos2) {
  const float xDist = powf(pos1.x - pos2.x, 2);
  const float yDist = powf(pos1.y - pos2.y, 2);
  return sqrtf(xDist + yDist);
}

inline float getNorm(vector2_T vector) {
  return sqrtf(powf(vector.x, 2) + powf(vector.y, 2));
}

inline vector2_T normalize(vector2_T vector) {
  const float norm = getNorm(vector);
  return scalarProductVec(vector, 1 / norm);
}

inline float getT(vector2_T A, vector2_T B, vector2_T C, vector2_T D) {
  return (((A.x - C.x) * (C.y - D.y) - (A.y - C.y) * (C.x - D.x)) /
          ((A.x - B.x) * (C.y - D.y) - (A.y - B.y) * (C.x - D.x)));
}

inline float getTDen(vector2_T A, vector2_T B, vector2_T C, vector2_T D) {
  return ((A.x - B.x) * (C.y - D.y) - (A.y - B.y) * (C.x - D.x));
}

inline float getU(vector2_T A, vector2_T B, vector2_T C, vector2_T D) {
  return (((A.x - C.x) * (A.y - B.y) - (A.y - C.y) * (A.x - B.x)) /
          ((A.x - B.x) * (C.y - D.y) - (A.y - B.y) * (C.x - D.x)));
}

inline float lineToPointDistance(vector2_T A, vector2_T B, vector2_T P) {
  const float num =
      fabsf((B.x - A.x) * (A.y - P.y) - (A.x - P.x) * (B.y - A.y));
  const float den =
      sqrtf((B.x - A.x) * (B.x - A.x) + (B.y - A.y) * (B.y - A.y));
  return num / den;
}

#endif