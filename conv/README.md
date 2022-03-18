## vednn support functions for convolution timing analyses

### Changes
- count_ops much faster (added idiv.hpp and hoist.hpp files)
- convolution feature `2<mb<=8` had typo, now it's `1<mb<=8`
