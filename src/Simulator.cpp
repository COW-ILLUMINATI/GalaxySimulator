#include <omp.h>
#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <random>
#include <list>
#include <vector>
#include <algorithm>
#include <string>
#include <SDL2/SDL.h>
#include <fstream>
#include <string>
class Vector3;

// Starts the clock
std::clock_t startTime = std::clock();

// Defines a pointer to the screen size (needs to be accessible everywhere, but the Vector3 class is still undefined!)
Vector3 *screenSize;

// Creates the window & renderer, will be assigned later
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

// Stolen from Stackoverflow, can't be bothered to make it myself... :P
void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
   const int32_t diameter = (radius * 2);
   int32_t x = (radius - 1);
   int32_t y = 0;
   int32_t tx = 1;
   int32_t ty = 1;
   int32_t error = (tx - diameter);
   while (x >= y)
   {
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);
      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }
      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}

// Oki, after this it's all code from me!!

// Big-ass Vector3 library
// Fairly self-explanatory
struct Vector3
{
    float x, y, z;
    Vector3(){x=0;y=0;z=0;}
    Vector3(float sx, float sy, float sz) {
        x=sx; y=sy; z=sz;
    }
    Vector3 operator+(Vector3 target){
        return Vector3(x+target.x,y+target.y,z+target.z);
    }
    Vector3 operator-(Vector3 target){
        return Vector3(x-target.x,y-target.y,z-target.z);
    }
    Vector3 operator*(float target){
        return Vector3(x*target,y*target,z*target);
    }
    Vector3 operator/(float target){
        return Vector3(x/target,y/target,z/target);
    }
    bool operator==(Vector3 target){
        return ((x == target.x) && (y == target.y) && (z == target.z));
    }
    bool operator!=(Vector3 target){
        return !((x == target.x) && (y == target.y) && (z == target.z));
    }
    std::string to_String(){
        return "( " + std::to_string(x) + " ; " + std::to_string(y)  + " ; " + std::to_string(z) + " )";
    }
    float Dot (Vector3 target) {
        return target.x*x + target.y*y + target.z*z;
    }
    Vector3 Cross (Vector3 target) {
        Vector3 tmp = Vector3(0,0,0);
        tmp.x = y * target.z - z * target.y;
        tmp.y = z * target.x - x * target.z;
        tmp.z = x * target.y - y * target.x;
        return tmp;
    }
    float SqrMagnitude () {
        return Vector3(x,y,z).Dot(Vector3(x,y,z));
    }
    Vector3 Normalized () {
        return Vector3(x,y,z) * (1.0/sqrt(Vector3(x,y,z).SqrMagnitude()));
    }
    Vector3 Rotate(Vector3 eulerAngles){ // Ghaaaaa
        float xx, yy, zz = 0;
        Vector3 tmpVector3 = Vector3 (x,y,z);
        xx = tmpVector3.x;
        yy = tmpVector3.y * cos(eulerAngles.x) - tmpVector3.z * sin(eulerAngles.x);
        zz = tmpVector3.y * sin(eulerAngles.x) + tmpVector3.z * cos(eulerAngles.x);
        tmpVector3.x=xx; tmpVector3.y = yy; tmpVector3.z=zz;
        xx = tmpVector3.x * cos(eulerAngles.y) + tmpVector3.z * sin(eulerAngles.y);
        yy = tmpVector3.y;
        zz = -tmpVector3.x * sin(eulerAngles.y) + tmpVector3.z * cos(eulerAngles.y);
        tmpVector3.x=xx; tmpVector3.y = yy; tmpVector3.z=zz;
        xx = tmpVector3.x * cos(eulerAngles.z) - tmpVector3.y * sin(eulerAngles.z);
        yy = tmpVector3.x * sin(eulerAngles.z) + tmpVector3.y * cos(eulerAngles.z);
        zz = tmpVector3.z;
        return Vector3 (xx, yy, zz);
    }

};

// Library to map 3D space to screen space ("Camera")
class Rasterizer {
    private:
        // Two screen axis for othographic projection (perspective is overkill)
        Vector3 projectionX = Vector3 (1, 0, 0);
        Vector3 projectionY = Vector3 (0, 1, 0);
        
        // Keeps track of the last color since switching is expensive AF
        Vector3 lastColor = Vector3 (255, 255, 255);

        // Rotation of the 'camera'
        Vector3 eulerAngles = Vector3 (-1, 0, -0.55f);

        // We use a function to check the active color (more readable this way)
        void ChangeColor(Vector3 newCol){
            if (newCol != lastColor){
                SDL_SetRenderDrawColor(renderer,newCol.x,newCol.y,newCol.z,255);
                lastColor = newCol;
            }
        }

        // This function takes 3D coordinates and maps them to the screen.
        Vector3 Project(Vector3 position, Vector3 projectionAxisX, Vector3 projectionAxisY){
            return ToScreenCoords(Vector3(  // Orthographic rendering, could be changed if needed
                position.Dot(projectionAxisX),
                position.Dot(projectionAxisY),
                0)
                );
        }
        // Converts [-1; 1] --> [Screenspace]
        // This makes it possible to resize the screen without wanting to commit suicide
        Vector3 ToScreenCoords(Vector3 fromRange){
            return Vector3((fromRange.x + 1) * screenSize->x / 2,(fromRange.y + 1) * screenSize->y / 2, 0);
        }          
    
    public:
        // Position of the camera
        Vector3 worldPosition = Vector3(0,0,0);

        // Low-level method for placing single 3D pixels on a 2D screen
        void PutPoint(Vector3 position, Vector3 color){
            position = position - worldPosition;
            // Projects them onto 2d
            position = Rasterizer::Project(position, projectionX.Rotate(eulerAngles), projectionY.Rotate(eulerAngles));   
            //Color
            ChangeColor(color);
            // Adds itself to the buffer
            SDL_RenderDrawPoint(renderer, position.x, position.y);
        }

        // Same, but for lines
        void PutLine(Vector3 start, Vector3 end, Vector3 color){
            start = start - worldPosition;
            end = end - worldPosition;
            
            // Projects them onto 2d
            start = Rasterizer::Project(start, projectionX.Rotate(eulerAngles), projectionY.Rotate(eulerAngles));
            end = Rasterizer::Project(end, projectionX.Rotate(eulerAngles), projectionY.Rotate(eulerAngles));            
            // Sets the color
            ChangeColor(color);
            // Adds itself to the buffer
            SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
        }

        // Same, with circles   (Note:: they will always face the camera!)
        void PutCircle(Vector3 center, float radius, Vector3 color){
            center = center - worldPosition;        
            // Projects them onto 2d
            center = Rasterizer::Project(center, projectionX.Rotate(eulerAngles), projectionY.Rotate(eulerAngles));            
            // Sets the color
            ChangeColor(color);
            // Adds itself to the buffer
            DrawCircle(renderer, center.x, center.y, radius);
        }

        // Rotates the camera
        void Rotate(Vector3 angle){
            eulerAngles = eulerAngles + angle;
        }
        
        // Zooming
        void Zoom(float scale) {
            projectionX = projectionX * scale;
            projectionY = projectionY * scale;
        }

        // Updates the aspect ratio
        void SetRatio(Vector3 scale){
            scale=scale.Normalized();
            projectionX = projectionX / scale.x;
            projectionY = projectionY / scale.y;
        }
};



// Stars!!!
struct Star
{
    Vector3 position = Vector3 (0,0,0);
    Vector3 velocity = Vector3 (0,0,0);
    Vector3 acceleration = Vector3 (0,0,0);
    
    Vector3 color = Vector3(0,0,0);
    float mass = 1;

    // Default constructor
    Star() {}

    // Called ever subframe
    void tick(float dt) {
        // Calc intégral lets gooo
        position = position + velocity * dt + acceleration * dt * dt * 0.5;        
        velocity = velocity + acceleration * dt;
        acceleration = Vector3 (0,0,0);
    }

    // Calculates the field at a point in space
    static Vector3 Solve(Vector3 location, Star b, float dt){    
        // avoids divisions by 0!
        if (location != b.position) {
            Vector3 r = b.position-location;
            //                                            \/-- distance cannot be smaller than the timestep! 
            float inv_n_r = 1.0/sqrt((r.SqrMagnitude() + dt));
            return r * inv_n_r * inv_n_r * inv_n_r * b.mass;
        }
        // If the position is the same as the object, the field is 0 at that point
        return Vector3(0,0,0);
    }

};

int main() {

    // Sets the screenSize pointer to an object in memory
    screenSize = new Vector3(0,0,0);

    // Loading stuff
    std::cout<< "Opening files.." <<std::endl;
    std::ifstream universe;
    universe.open("Gal_data/Universe");
    std::ifstream uiSettings;
    uiSettings.open("Gal_data/UI_Settings");
    std::ifstream simSettings;
    simSettings.open("Gal_data/Sim_Settings");

    // UI
    std::cout<< "UI settings" <<std::endl;
    // Grabs the screen size
    uiSettings >> screenSize->x;
    uiSettings >> screenSize->y;
    // Grabs the target framerate
    int tfps;
    uiSettings >> tfps;
    uiSettings.close();
    

    // Sim
    std::cout<< "Simulator settings" <<std::endl;
    float timestep;
    int budget;
    int mode;
    // Gets the timestep
    simSettings >> timestep;
    simSettings >> budget;
    // Gets the 'mode', either 0 = full simulation, -1 = significant only, anything else = limited simulation.
    simSettings >> mode;
    simSettings.close();


    // Stars
    std::cout<< "Stars.." <<std::endl;
    int starCount;
    // Gets the TOTAL amount of objects
    universe >> starCount;
    int sigStars;
    // Gets the significant stars
    universe >> sigStars;
    
    // Quick log
    std::cout<< "Found stars: " << starCount << std::endl;
    
    // Creates the vector (I hear these are better than arrays, might change it if I get time to do benchmarks)
    std::vector <Star> stars;
    stars.reserve(starCount);
    
    // Itterator for array access
    int i = 0; // THIS STUPID INITIALIZATION CAUSED 12 FUCKING HOURS OF DEBUGGING OVER DISCORD GHAA KILL ME 

    while (true)
    {
        // Logs the currently parsed object
        std::cout<< "Star: " << i << "\r";
        
        // If we're done, we can exit
        if( universe.eof() ) break;
        
        // Adds cell to the vector
        stars.push_back(Star());

        // Positions
        universe >> stars[i].position.x;
        universe >> stars[i].position.y;
        universe >> stars[i].position.z;

        // Velocities
        universe >> stars[i].velocity.x;
        universe >> stars[i].velocity.y;
        universe >> stars[i].velocity.z;

        // Mass
        universe >> stars[i].mass;
        if (i >= sigStars){
            // Ignores black holes
            // Ex.: If a galaxy has a mass of 200, and 10 stars are parsed, it'll act like those stars had a mass of 20 (x10)
            stars[i].mass=stars[i].mass/budget;
        }

        // Color
        universe >> stars[i].color.x;
        universe >> stars[i].color.y;
        universe >> stars[i].color.z;

        // At the end, since we want to start at 0
        i ++;
    }

    // Sets the mode
    // Acts as an override to starCount, where it stops the parser at a given point during the process
    // This allows the planning of camera motions
    if (mode == 1){
        starCount = sigStars;
        budget = sigStars;
    }

    // SDL output
    std::cout << std::endl << "Bodies generated!" << std::endl;
    std::cout << "SDL init : " << SDL_Init(SDL_INIT_VIDEO) << std::endl;
    std::cout << "SDL Wind : " << SDL_CreateWindowAndRenderer(screenSize->x, screenSize->y, 0, &window,&renderer) << std::endl;
    std::cout << "SDL size : " << SDL_RenderSetScale(renderer,1,1) << std::endl;
    std::cout << std::endl;

    // Spawns a input listener
    SDL_Event event;

    // Spawns the camera
    Rasterizer rasterizer;
    rasterizer.SetRatio(*screenSize);
    
    // Makes sure the sim starts stopped
    bool playing = false;

    // Starts the deltaTime system
    std::clock_t prevClock = startTime;
    bool pushframe = false;

    // Camera will be centered on this star
    int trackedStar = 0;

    // Initialize the camera position
    Vector3 cameraPosition = Vector3(0,0,0);

    // Predeclaration of the parser array
    Star *parseStars[budget];

    while (true) {

        // Should the next frame be pushed to the renderer?
        pushframe = (std::clock() - prevClock) / (float)CLOCKS_PER_SEC > (1.0/tfps);

        if (pushframe) {
            // Starts the frame
            SDL_SetRenderDrawColor(renderer,0,0,0,255);
            SDL_RenderClear(renderer);

            // Centers camera
            rasterizer.worldPosition = cameraPosition;
            
            // Draws the grid
            rasterizer.PutLine(Vector3(-10000,0,0),Vector3(10000,0,0),Vector3(100,0,0));
            rasterizer.PutLine(Vector3(0,-10000,0),Vector3(0,10000,0),Vector3(0,100,0));
            rasterizer.PutLine(Vector3(0,0,-10000),Vector3(0,0,10000),Vector3(0,0,100));

            // Draws the origin
            rasterizer.PutLine(Vector3(0,0,0),Vector3(1,0,0),Vector3(255,0,0));
            rasterizer.PutLine(Vector3(0,0,0),Vector3(0,1,0),Vector3(0,255,0));
            rasterizer.PutLine(Vector3(0,0,0),Vector3(0,0,1),Vector3(0,0,255));

        }


        if (playing){

            // Which stars will be parsed this frame?
            // Picks them at random -- this appears stupid but is actually __very__ accurate
            for (int i = 0 ; i < budget ; i ++){
                parseStars[i] = &stars[ (rand() % (starCount - sigStars)) + sigStars];
            }
            
            // Adds the black holes to the stack
            for (int i = 0 ; i < sigStars ; i ++) {
                parseStars[i]=&stars[i];
            }

            // Can't be fucked to parallelize myself
            #pragma omp parallel for collapse(2)
            // Each star vs budget
            for (int i = 0 ; i < starCount ; i++) {
                for (int j = 0 ; j < budget ; j++){
                    
                    // Solves the new acceleration for each star
                    stars[i].acceleration = stars[i].acceleration + 
                        (
                            Star::Solve(stars[i].position, *parseStars[j], timestep)
                        ) 
                        ;
                }
            }

            // Must update everything at the same time
            #pragma omp parallel for
            for (int i = 0 ; i < starCount ; i++) {
                stars[i].tick(timestep);
            }


	        // Camera motion (on ticks instead of frames, better camera planning):
	        rasterizer.Rotate(Vector3(0,0,0.007f * timestep));

        }

        
        // Draws
        if (pushframe) {
            // Every star -- cannot be threaded! (bottleneck)
            for (int i = 0 ; i < starCount ; i++) {
                if (i < sigStars) {
                    // Black holes are highlighted
                    rasterizer.PutCircle(stars[i].position, 2 , stars[i].color);
                } else {
                    rasterizer.PutPoint(stars[i].position, stars[i].color);
                }
            }
            
            // New position
            cameraPosition = stars[trackedStar].position;
            
            // Pushes the frame
            SDL_RenderPresent(renderer);
            
            // Updates the time
            prevClock = std::clock();
        }

        // Queries the event listener
        // ''While'' to clear the buffer
        while(SDL_PollEvent( &event )){
            // 'What just happened?'
            switch( event.type ){
                case SDL_KEYDOWN:
                
                    switch (event.key.keysym.sym)
                    {
                        // Camera block
                        case SDLK_LEFT:
                            rasterizer.Rotate(Vector3(0,0,0.05f));
                            break;
                        case SDLK_RIGHT:
                            rasterizer.Rotate(Vector3(0,0,-0.05f));
                            break;
                        case SDLK_UP:
                            rasterizer.Rotate(Vector3(0.05f,0,0));
                            break;
                        case SDLK_DOWN:
                            rasterizer.Rotate(Vector3(-0.05f,0,0));
                            break;
                        case SDLK_PAGEDOWN:
                            rasterizer.Zoom(1/1.1f);
                            break;
                        case SDLK_PAGEUP:
                            rasterizer.Zoom(1.1f);
                            break;
                        case SDLK_SPACE:
                            playing = !playing;
                            break;
                        case SDLK_RETURN:
                            trackedStar = rand() % starCount;
                            break;
                    }
                    break;
                case SDL_QUIT:
                    return 0;
                    break;
                default:
                    break;
            }
        }    

    }
    return 0;
}
