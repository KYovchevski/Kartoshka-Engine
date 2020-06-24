# Kartoshka Engine
 
Kartoshka engine is a project I have been working on for the past 2 months in my spare time from school. The goal of the project was for me to gain experience with Vulkan and use it as a sandbox for other rendering techniques in the future, such as physically based rendering and animations. I started working on this project with no knowledge of Vulkan, and the final result has been achieved in around 130-140 hours of research and programming.
The final product is capable of rendering Sponza with very few visual glitches, with a single point light whose position and colour can be controled via ImGui. While Sponza is used by the project for showcase, other models and scenes can be loaded from GLTF files. The rendering pipeline ins't advanced, and currently supports only a single point light with normal mapping for the surfaces.

# Images

![Render of the scene](https://github.com/KYovchevski/Kartoshka-Engine/tree/master/Assets/Images/sponza.jpg)
![Render of the lion head in Sponza](https://github.com/KYovchevski/Kartoshka-Engine/tree/master/Assets/Images/sponzaLion.jpg)

# Known issues
## Vulkan errors when exiting 
One of the issues with the current state of the project is that upon exiting the application by closing the window, the Vulkan validation layer throws errors. These errors are caused because the Vulkan objects are being deleted in the wrong order, most notable the VkDevice is destroyed before items such as VkBuffers.

## No transparency
There are a few meshes in Sponza which make use of transparency, for example the chains used for the hanging pots. Those meshes aren't properly transparent since blending isn't enabled for the graphics pipeline.
