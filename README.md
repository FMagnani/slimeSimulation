# Agent-Based Slime Simulation
The final result is not yet a perfect slime, but it was the original inspiration for this project. In any case, there are some slimey behaviours performed by the simulation so the name has been kept. Other patterns are stripey, similar to the cover of Unknown Pleasures. Some webby stuff can emerge, similar to a mycelium network :pray:. Also, I found blobby behaviour, both in the form of a big chill ball of light or many small vortices of spermatozoa.  
  
**Slimey**

**Stripey**

**Webby**

**Big Blobby**

**Lil Blobby**


## Algorithm
Agent-based models are both scientifically important and graphically pleasing. I suggest to watch [the video on Ants and Slimes](https://youtu.be/X-iSQQgOd1A) of Sebastian Lague for an introduction to the concept from a coding perspective. Often, agent-base models give their best when *a lot* of agents are introduced in the simulation at the same time. In such a case from a particle simulation you start having a sort of fluid simulation, that is inherently pleasing to see. Parallelization of the computation over the GPU (i.e. the use of shaders) is needed to overcome this computational challenge.  
Let's briefly explain the idea of the algorithm. Imagine a lot of people walking around, that drop a little rock behind at every step. They have a limited field of view and at each step they steer a bit in the direction in which they see more rocks. Also, the rocks are actually ice cubes and they vanish as time passes. That's basically the idea. In fact, every agent interacts with the field (dropping and counting rocks) rather than directly to all the other agents. More precisely, the rocks are the quantity of white stored in every pixel (we call them "trails"). It fades a bit from white to black at every frame. Also, blurring is applied at every frame in order to represent a sort of spatial diffusion of the trails. Each agent, at each frame, does the following three actions: they update their position through the velocity (by an amount set by deltaTime), then they *sense* the trails at specific locations (set by sensorAngle and sensorDst), finally they update their speed (htat will be applied to the position in the next frame) *steering* by an amount set by turnSpeed.  
**Sensing** The agents sense the trails at three directions: in front of them (their direction) and simmetrically at their left and right (by an angle set by sensorAngle). The distance at which the sensing happens is set by sensorDst, that is not their field of view. They always sense the trails by averaging the trails over a square centered at the sensing position.  
**Steering** After having sensed the trails at three positions, the agents have three weights: forwardWeight, leftWeight and rightWeight. The steering is not continuous, like having forces in the three directions that are vectorially combined (I tried, it's more difficult than this). It is *conditional*: the compare the three weights and they steer in the winning direction, by an amount set by turnSpeed.

## Algorithm and parameters
| Parameter | Meaning | Potential effect (vague) |
|--------------|-----------|-----------|
| fps | Display the current fps at which the application runs. | It cannot be changed. It is useful to the user in order to estimate the effort currently made by the copmuter. In general, if the fps are still high, the number of particles can be increased. |
| deltaTime | This multiplies the velocity in order to get the new position in the simulation. It doesn't contribute to the steering of the agents, so in practice it's the time interval at which the agents sense the field and steer. If it's large, between a sensing and the next the agents move in the same direction. If it's small, the agents cover few space before to sense and steer again. | It's somehow similar to increasing the inertia of the agents of going in the same direction |
| decayRate | The speed at which the  white trails vanish over time. If it's 0, they never fade. | Decreasing this parameter can "freeze" the current routes but also creating new ones. Graphically, a small decay rate reduces the granularity of the image. A high decay rate gives a greater effect of "particolate" or "sand". |
| turnSpeed | The amount of steering in the direction at which the agent senses more trail. If it's 1, the agent velocity is rotated to the sensor direction. | Setting this to 0 completely remove the steering, so it can be used to mix up the layout, to randomize the situation. Setting this suddenly to 1 makes the pattern to emerge from chaos in few instants. |
| sensorAngle | The sensing is made in two directions: the current direction of the agent plus or minus the sensorAngle. If it's 0, the sensing is not made since they check the same point in front of them and they cannot decide where to steer. If it's pi/2 they sense precisely at their left and right (but they can steer less than this, depending on turnSpeed). | It's somehow related to the radius of bending of the routes followed by the agents. For example, if this is pi/2 and turnSpeed is 1, it's likely that the agents organize in many circles. If it's low, near to 0, the agents more likely organize into stripes. |
| sensorDst | The distance at which the agents sense. This is not their field of view. They always sense the trails in a 3x3 or 5x5 kernel. sensorDst selects how much distant this sensing is made. | I believe that this is somehow related to the scale of the final slime. If this is small, the image is made of many small branches/blobs. If this is large, the image is made up of few very big branches/blobs. |
| sensorSize | It's the size of the sensor. If it's 1 the sensor is 3x3, if it't 2 the sensor is 5x5. | I didn't explore the possibility of changing this, since it makes the frame rate to drop when there are many particles. Anyhow, I believe that the difference is tiny with images of large sizes, maybe it's important in small-scale implementations of this model. |
| start | Pause completely the application. | It's useful for changing more parameters at the same instant or to set the initial parameters. It has the same effect of setting deltaTime to 0, but it's instantaneous. |

## Frame rate and number of agents
Looking at the frame rate it's possible to get an idea of the computational effort of the simulation. If the frame rate is still high, you could evaluate the possibility of increasing the number of particles of the simulation.  
I lack a complete understanding of parallel computation over GPU, so the next ideas could be (I hope only partially) wrong. I understood the business as dividing a very long *for* cycle (N iterations) into P parallel blocks, each one of S sequential *for* iterations. Clearly N = P x S. Thus, if I have 2000 particles I can run 200 parallel threads, each that iterates sequentially over 10 of them.  
There it is a maximum number of threads that your GPU can support. The number of parallel threads that you ask it to carry on is specified in ``compute.dispatchCompute(1024, 1, 1);`` in *ofApp.cpp*. In this case they are 1024. These are made *in parallel*. Each one of these threads still performs *sequentially* some operations, their number is specified in ``layout(local_size_x = 100, local_size_y = 1, local_size_z = 1) in;`` in *compute.glsl*. In this case each thread personally iterates over 100 particles. Since there are 1024 threads, the total number of particles simulated are 1024 x 100. This number must match the line ``#define NPARTICLES 102400`` in *ofApp.h*. In fact, the shader is the code to be executed on each single thread.  
I apologize with the experts of the field for the loose and maybe wrong interpretation. Let me know dangerous errors if you spot them.


## Overview of the code
Let's consider the case of Boids, in which the agents steer depending on their relative position. The interaction of $N$ agents to $N$ agents increases as $N^2$ (and $N$ should be the biggest possible), so I believe that even in this case a field that mediates the interactions can highly reduce the computational cost. You could imagine that if each agent only interacts with his $k$ nearest neighbors, the number of interactions is only $kN$, but note that you still have to compute the distance of the agent to all the others in order to find his nearest neighbors (I believe that smarter algorithms exist to do this, but I don't know them). Also, you could divide your field in sectors and make a separate computation for each one, but still you have to ask to every agent his position in order to group them. Moreover, maybe you want the agents to have a directional field of view and I think that a mediating field is useful even for that. There it is also [a video on Boids](https://www.youtube.com/watch?v=bqtqltqcQhw) by Sebastian Lague.  
Therefore, broadly speaking, we have to deal with the **position data**, the **velocity data** and the **field data**. The position and velocity data are best represented by arrays of vectors while I decided to store the field data into a texture, i.e. an image. In practice, in the code I employ a [*frame buffer object*](https://openframeworks.cc/documentation/gl/ofFbo/), that is a sort of buffer for textures, since it has to be updated at each frame. I rely on two shaders: *compute* and *blur*. *compute* is a compute shader and it iterates (maybe it's better to say "operates", since its computation is partially parallelized) over the agents. *blur* is a classic shader that iterates (operates) over the pixels of the image.  
  
Conceptually the steps to do are these:  
- **1** Update the velocity of each agent with the image  
- **2** Update the position of each agent with the velocity  
- **3** Update the image with the positions of all the agents  
- **4** Blur and decay the image  
- **5** Go to 1  
  
In practice:  
- **1A** Compute shader reads the image and updates the velocity of each agent  
- **1B** Compute shader also updates the position of each agent with its velocity  
- **2A** Image is updated by the velocity (by drawing the set of vertices *vbo*)  
- **2B** Decay is implemented on the image  
- (The steps 2A and 2B are saved into the first frame buffer)  
- **3** Blur shader is applied to the image (in the second frame buffer)  
- **4** Ping pong is implemented for the three data structures  

## Ping Pong
I implemented differently the ping pong for the positions, the velocities and the image.  
The two position buffers are posGpuPing and posGpuPong and I simply copy the first into the second after the update step. I did so because the vertex buffer is linked to posGpuPing (maybe it's ok to change this linking at each frame, but I did't try).  
The ping pong of the two velocity buffers is implemented by changing at each frame their binding with the GPU.  
The ping pong of the frame buffers is implemented by using a dedicated pingPongBuffer struct that I found in various examples of OpenFrameworks.  

## Other possible implementations
I really tried a number of different possibilities but this is **the only one** which actually works. I'm new in the field of graphics and graphical interfaces so I have no idea of why this is the case.  
Initially, I tried to employ no compute shaders. The data for position and velocity was stored in textures which were updated by dedicated classic shaders (of the kind of *blur*, here). There were problems, maybe since the textures are capped at 255 (or at 1? Maybe this depends on their internal format? How can I check?) or maybe for a different reason that I don't manage to imagine.  
In a second period, I tries to employ two compute shaders. The first (compute), iterating over the agents, for reading the read-only image, updating the velocities and the positions, and finally updating the write-only copy of the image by accessing the correct pixel (the index of the pixel is computed using the position). The second compute shader iterated over the pixel of the image, implementing blur and decay. None of the two shaders never worked in a good way. I also tried to store the data of the image into a normal array, in order not to have to deal with the internal format of the textures. Such array would have been used to fill a texture later. Still, I did it wrong for some reason.  
At the end, this is the only possibility that worked for me:  
- A compute shader and a "classic" shader (I apologize for the probably wrong name)  
- Two buffer objects for the position and two for the velocity (I'm sure that they could be packed into a single struct *Agent*)  
- A *vbo* (vertex buffer object) for displaying the positions  
- Two *fbo*s (frame buffer object) for storing and updating the field/image  
  
If there are other ways of implementation, please tell me. They would be highly informative to me.  

## Resources
I got inspiration from [this video](https://youtu.be/X-iSQQgOd1A) of Sebastian Lague. He makes it using Unity, but [his script](https://github.com/SebLague/Slime-Simulation/blob/main/Assets/Scripts/Slime/SlimeSim.compute) have been useful for understanding the algorithm.  
I employ [Open Frameworks](https://openframeworks.cc/). A good path for getting at this level of understanding could be given by the following resources:  
- This [introduction to the concept of shader](https://openframeworks.cc/ofBook/chapters/shaders.html) and the corresponding examples  
- This [example of a particle system](https://github.com/openframeworks/openFrameworks/tree/master/examples/gl/gpuParticleSystemExample), using shaders applied to textures  
- This [example of boids](https://github.com/openframeworks/openFrameworks/tree/master/examples/gl/computeShaderParticlesExample), using compute shaders  
- This [example](https://github.com/openframeworks/openFrameworks/tree/master/examples/gl/fboTrailsExample) for implementing the trail map
- This good [tutorial on compute shaders](https://maciejmatyka.blogspot.com/2022/01/compute-shaders-in-open-frameworks.html) of Maciej Matyka
