CSCE 489 Computer Animation P3 Report
Aaron Sanchez
The input data was downloaded from facewaretech.com
All tasks completed

Incompatible Blendshapes: 
We cannot use two blendshapes that operate on the same area of the mesh, as this will cause conflicts of how far the mesh actually moves.
Let's use the head blendshape as an example, once you use one blendshape on the mouth to make it smile, and another to make the mouth yawn (open up wide)
this creates problems, as we are saying that our new position x(t) = our original position x(0) + weight 1 * difference between blend1 and original position, 
and we are adding this to the weight2 * difference between blend2 and original position. So if both are affecting the mouth region, meaning that the delta position
for both is greater than 0, we are going to move weight1 * delta1 + weight2 * delta2, which is going to make us move weird amounts, that are not what we wanted 
for this example. It is going to make the lower lips go down, unlike a regular smile, but similar to a yawn, and the top lips would move up twice as much as they 
usually would, as both the smile and yawn move the lips up, this could cause the positions to go through the nose, or into a weird position causing a not as 
good looking animation.