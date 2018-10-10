### loops playground
- very simple prototype for loop fusion.
  - fuse2 -h     help
  - fuse2 -u     shows suggestive asm-like output for unrolling loop

fuse2lin considers calculating a linear combination of vectorized loop indices.
But there is not much opportunity for optimizing the general case, I think.
So it is not very interesting at this point!
(Better to see if a multiple of inner-loop length is "almost 256")

