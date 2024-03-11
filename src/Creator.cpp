// some good params for the collision:
// G1: p[0,0,0], v[0,0,0]
// G2: p[250,0,250], v[0,-0.5,0]
// In general, it's good to have the velocity be perpendicular to the positions








#include <time.h>
#include <iostream>
#include <math.h> 
#include <string>
#include <fstream>
using namespace std;

string ask(string prompt){
    string answer;
    cout << prompt << endl;
    cin >> answer; 
    return answer;
}

int askInt(string prompt){
    int answer;
    cout << prompt << endl;
    cin >> answer; 
    return answer;
}
float askFloat(string prompt){
    float answer;
    cout << prompt << endl;
    cin >> answer; 
    return answer;
}



float q_rsqrt(float number)
{
  long i;
  float x2, y;
  const float threehalfs = 1.5F;

  x2 = number * 0.5F;
  y  = number;
  i  = * ( long * ) &y;                       // evil floating point bit level hacking
  i  = 0x5f3759df - ( i >> 1 );               // what the fuck?
  y  = * ( float * ) &i;
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
  y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

  return y;
}


// Big-ass Vector3 library (naming is not confusing!!! ghaaa)
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
        return Vector3(x,y,z) * q_rsqrt(Vector3(x,y,z).SqrMagnitude());
    }
    Vector3 Rotate(Vector3 eulerAngles){
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


int main () {

    ofstream fileEditor;


    char choice;
        cout << "---------- Profile Editor ----------" << endl;
        cout << "Galaxy Creation................. [a]" << endl;
        cout << "Simulator Settings.............. [b]" << endl;
        cout << "UI Settings..................... [c]" << endl;
        cout << "Prepare Universe................ [d]" << endl;
        cout << endl << endl<< endl<< endl<< endl;
        cout << "Simulator loads Gal_data/Universe" << endl;
    cin >> choice; 
    
    if (choice == 'a'){
        //Galaxy creator
        cout << "------------- UI settings ----------" << endl;
        fileEditor.open ("Gal_data/Galaxy_" + ask ("Galaxy name? "));


        int seed = askInt("Seed (0 for random)?           [   0] ");
        if ( seed == 0 ) {seed = time(NULL);}
        srand (seed);

        int sigStars      = askInt("Significant Stars?    [ 100 ] ");
        int massBH        = askInt("Central Mass?         [ 100 ] ");
        float massStars     = askFloat("Other Masses?         [ 0.1] ");
        int otherStars    = askInt("Star Count?           [10000] ");
        int radius        = askInt("Radius?               [ 100 ] ");
        float zDistribution = askFloat("Z Distribution (%)?   [ 0.1 ] ");
        int colorCount    = askInt("Color Count?          [  8  ] ");

        /*  
            starCount
            sigStars
            {
                x
                y
                z
                vx
                vy
                vz
                mass
                colorx
                colory
                colorz
            }
        */

        fileEditor << sigStars + otherStars<< endl;
        fileEditor << sigStars<< endl;

        fileEditor << 0<< endl;
        fileEditor << 0<< endl;
        fileEditor << 0<< endl;
        fileEditor << 0<< endl;
        fileEditor << 0<< endl;
        fileEditor << 0<< endl;
        fileEditor << massBH<< endl;
        fileEditor << 255<< endl;
        fileEditor << 255<< endl;
        fileEditor << 255<< endl;

        for (int i = 0 ; i < sigStars ; i++) {
            printf("Generating body pass 1 %10d/%d\r",i+1, sigStars);
            Vector3 posTmp = Vector3(
                                        rand()%10000/10000.0 * radius - radius/2,
                                        rand()%10000/10000.0 * radius - radius/2,
                                       (rand()%10000/10000.0 * radius - radius/2) * zDistribution
                                    );
            fileEditor << posTmp.x<< endl;
            fileEditor << posTmp.y<< endl;
            fileEditor << posTmp.z<< endl;

            Vector3 velTmp = posTmp.Cross(Vector3(0,0,1)).Normalized() * sqrt(massBH/sqrt(posTmp.SqrMagnitude()));
            fileEditor << velTmp.x<< endl;
            fileEditor << velTmp.y<< endl;
            fileEditor << velTmp.z<< endl;

            fileEditor << massStars<< endl;

            fileEditor << 255<< endl;
            fileEditor << 255<< endl;
            fileEditor << 255<< endl;
        }

        cout << endl;

        Vector3 lastColor = Vector3(random()%64+128,random()%128+128,random()%128+128);
        for (int i = 0 ; i < otherStars ; i++) {
            printf("Generating body pass 2 %10d/%d\r",i+1, otherStars);
            Vector3 posTmp = Vector3(
                                        rand()%10000/10000.0 * radius - radius/2,
                                        rand()%10000/10000.0 * radius - radius/2,
                                       (rand()%10000/10000.0 * radius - radius/2) * zDistribution
                                    );
            fileEditor << posTmp.x<< endl;
            fileEditor << posTmp.y<< endl;
            fileEditor << posTmp.z<< endl;

            Vector3 velTmp = posTmp.Cross(Vector3(0,0,1)).Normalized() * sqrt(massBH/sqrt(posTmp.SqrMagnitude()));
            fileEditor << velTmp.x<< endl;
            fileEditor << velTmp.y<< endl;
            fileEditor << velTmp.z<< endl;

            fileEditor << 0 << endl;

            if ((int)(i % ((int)(otherStars/colorCount))) == 0) {
                lastColor = Vector3(random()%64+128,random()%128+128,random()%128+128);
            }

            fileEditor << lastColor.x<< endl;
            fileEditor << lastColor.y<< endl;
            fileEditor << lastColor.z<< endl;
        }

    } else if (choice == 'b'){
        // Simulator settings
        cout << "-------- Simulator Settings --------" << endl;
        cout << "(Universes will require recreation! )" << endl;
        fileEditor.open ("Gal_data/Sim_Settings");
        fileEditor << ask("Timestep?                       [0.01] ") << endl;
        fileEditor << ask("Mode? [0 = full; -1 = sig; else = fix] ") << endl;

    } else if (choice == 'c'){
        // UI settings
        cout << "------------- UI settings ----------" << endl;
        fileEditor.open ("Gal_data/UI_Settings");
    
        fileEditor << ask("Screen size x?    [1920] ") << endl;
        fileEditor << ask("Screen size y?    [1080] ") << endl;
        fileEditor << ask("Target framerate? [ 24 ] ") << endl;
    
    } else if (choice == 'd') {
        cout << "--------- Prepare Universe ---------" << endl;
        fileEditor.open ("Gal_data/Universe");
        int galCount = askInt("Galaxy amount? ");
        ifstream galaxies[galCount];
        int sigStars[galCount];
        int starCount[galCount];
        Vector3 offset[galCount];
        Vector3 angle[galCount];
        Vector3 velocity[galCount];

        char comma;

        for (int i = 0 ; i < galCount ; i ++){
            galaxies[i].open(("Gal_data/Galaxy_" + ask ("Galaxy a name? ")));
            cout << "Position offset [x,y,z]" << endl;
            cin >> offset[i].x >> comma >> offset[i].y >> comma >> offset[i].z;
            cout << "Angle (radians) [x,y,z]" << endl;
            cin >> angle[i].x >> comma >> angle[i].y >> comma >> angle[i].z;
            cout << "Velocity [x,y,z]" << endl;
            cin >> velocity[i].x >> comma >> velocity[i].y >> comma >> velocity[i].z;
        }

        /*  
            starCount + starCount
            sigStars + sigStars
            2 x {
                x
                y
                z
                vx
                vy
                vz
                mass
                colorx
                colory
                colorz
            }
        */

        // StarCount
        int tmpStarCount = 0;        
        for (int i=0;i<galCount;i++){
            galaxies[i] >> starCount[i];
            tmpStarCount += starCount[i];
        }
        fileEditor << tmpStarCount << endl;
        
        // SigStars
        tmpStarCount=0;
        for (int i=0;i<galCount;i++){
            galaxies[i] >> sigStars[i];
            tmpStarCount += sigStars[i];
        }
        fileEditor << tmpStarCount << endl;

        // Sequential to preserve contiguity
        float tmp=0;
        Vector3 vtmp;
        // Sigstars
        for (int i = 0 ; i < galCount; i ++){
            for (int j = 0 ; j < sigStars[i]; j ++){            
                // Pos
                galaxies[i]>>vtmp.x;
                galaxies[i]>>vtmp.y;
                galaxies[i]>>vtmp.z;
                vtmp = vtmp.Rotate(angle[i]) + offset[i];
                fileEditor<<vtmp.x<< endl;
                fileEditor<<vtmp.y<< endl;
                fileEditor<<vtmp.z<< endl;
                
                // Vel
                galaxies[i]>>vtmp.x;
                galaxies[i]>>vtmp.y;
                galaxies[i]>>vtmp.z;
                vtmp = vtmp.Rotate(angle[i]) + velocity[i];
                fileEditor<<vtmp.x<< endl;
                fileEditor<<vtmp.y<< endl;
                fileEditor<<vtmp.z<< endl;
                
                // Mass
                galaxies[i]>>tmp;
                fileEditor<<tmp<< endl;

                // Color
                galaxies[i]>>vtmp.x;
                galaxies[i]>>vtmp.y;
                galaxies[i]>>vtmp.z;
                fileEditor<<vtmp.x<< endl;
                fileEditor<<vtmp.y<< endl;
                fileEditor<<vtmp.z<< endl;
            }    
        }
        // other stars:
        for (int i = 0 ; i < galCount; i ++){
            for (int j = 0 ; j < starCount[i] - sigStars[i]; j ++){            
                // Pos
                galaxies[i]>>vtmp.x;
                galaxies[i]>>vtmp.y;
                galaxies[i]>>vtmp.z;
                vtmp = vtmp.Rotate(angle[i]) + offset[i];
                fileEditor<<vtmp.x<< endl;
                fileEditor<<vtmp.y<< endl;
                fileEditor<<vtmp.z<< endl;
                
                // Vel
                galaxies[i]>>vtmp.x;
                galaxies[i]>>vtmp.y;
                galaxies[i]>>vtmp.z;
                vtmp = vtmp.Rotate(angle[i]) + velocity[i];
                fileEditor<<vtmp.x<< endl;
                fileEditor<<vtmp.y<< endl;
                fileEditor<<vtmp.z<< endl;
                
                // Mass
                galaxies[i]>>tmp;
                fileEditor<<tmp<< endl;

                // Color
                galaxies[i]>>vtmp.x;
                galaxies[i]>>vtmp.y;
                galaxies[i]>>vtmp.z;
                fileEditor<<vtmp.x<< endl;
                fileEditor<<vtmp.y<< endl;
                fileEditor<<vtmp.z<< endl;
            }    
        }

    }

    fileEditor.close();
    return 0;
}



