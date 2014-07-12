#ifndef UTILS_H_
#define UTILS_H_

// It's gonna be clamp this clamp that, badda climp badda clamp, they won't know what clamped them!
#define CLAMP(value, lower_bound, upper_bound) \
((value) < (lower_bound)? (lower_bound) : ((value) > (upper_bound) ? (upper_bound) : (value)))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#endif /* UTILS_H_ */
