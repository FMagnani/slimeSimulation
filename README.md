# Agent-Based Slime Simulation

images

## Algorithm and parameters

## Frame rate and number of agents


## Agent based model
Agent-based models are both scientifically important and graphically pleasing. I suggest to watch [the video on Ants and Slimes](https://youtu.be/X-iSQQgOd1A) of Sebastian Lague for an introduction to the concept from a coding perspective. Often, agent-base models give their best when *a lot* of agents are introduced in the simulation at the same time. In such a case from a particle simulation you start having a sort of fluid simulation, that is inherently pleasing to see. Parallelization of the computatino over the GPU (i.e. the use of shaders) is needed to overcome this computational challenge.  

## Overview of the code
Let's briefly explain the idea of the algorithm. Imagine a lot of people walking around, that drop a little rock behind at every step. They have a limited field of view and each step, they steer a bit in the direction in which they see more rocks. Also, the rocks are actually ice cubes and they vanish as time passes. That's basically the idea. In fact, every agent interacts with the field (dropping and counting rocks) rather than directly to all the other agents.  
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
