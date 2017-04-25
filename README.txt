
Modules:

1. Vector3: It’s contains 3 double fields.I use this to represent coordinates, the color made of RGB. I also have implemented the addition, multiplication and dot multiplication.
 
2. Ray: It’s a ray contains a origin point and a direction. Origin point is defined by Vector3, as well as the direction. This class allows get point on the ray. For example, （Vector3 getPoint(double t), where t is the distance from origin to the point. 

3. Object: it’s the base class to describe an object. It contains 3 fields: center, color, lambert. It has function getNormal and isIntersected. get normal will return the normal vector on the vector. isIntersected(Ray ray, double& distance) will pass in a ray as argument. And it will determine if this ray is intersected to this object. and it will store the distance from intersection and origin to “distance“.

4. Sphere: it inherits from Object. It has radius parameter to represent the radius of the sphere. It overloads the getNormal and isIntersected.

5. Plane: it inherits from Object to represent the plane. It has normal parameter to represent the normal vector of the plane. It overloads the getNormal and isIntersected as well.

6. World: World module is my controller. It describes how many objects in the scene and how to generate the image. It has the objects vector to represent all the objects in the scene. Lights vector represents the light in the scene. Camera is the location of the camera. length_x and length_y represent the pixels of the figure. I also use buffer to represent the RGB color of each pixel. 
When world is called, it will initialize objects, lights, camera and so on. Then it will set the buffer[i][j](the color of each pixel). During the rendering process, it first will  generate a ray from camera to the pixel of the figure. Then it will determine the the object that the ray encounter. Then, it will iterate all the light source and determine the influence of the light source on the pixel color using the formula. It will repeat this process until it go through all the pixels.

7. parser: it simply parses the file and do the error handling.   

How to achieve concurrency?
We create a couple thread, and each thread will calculate the color of part of 
the final image. For example, if we gonna to generate a 256*256 image and we have
4 thread. The first thread will generate the first 64 line of pixels of the image.
The second thread will generate the second 64 line of pixels of the image.For n threads, each thread will take care of 1/n of figure. 