/*
      Self-Shadows

Step 1 : Precompute n slices of the yarn -> store in 3D texture ordered by the angle of incidence of the light (== rotation of the yarn if the light stays still) :
    - [0°:(2D Texture), 20°:(2D Texture), 40°:(2D Texture), ...,  2*PI/plyCount:(2D Texture)]
Step 2 : Inside the fragment shader, get the light direction relative to the yarn orientation
Step 3 : Get the point to shade expressed in the space expressed in Step 2.
Step 3 : Sample the 3D texture based on the light direction / orientation (interpolate between the most suited slices) and the point we want to shade.
         Sample

*/