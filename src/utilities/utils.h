#ifndef UTILS_H_
#define UTILS_H_

// It's gonna be clamp this clamp that, badda climp badda clamp, they won't know what clamped them!
#define CLAMP(value, upper_bound, lower_bound) \
do{ \
	if(value > upper_bound) \
		value = upper_bound; \
	if(value < lower_bound) \
		value = lower_bound; \
} while (0)


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))


#endif /* UTILS_H_ */
