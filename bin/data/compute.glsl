#version 440

layout(binding=0) buffer posPing{
    vec2 posWrite[];
};
layout(binding=1) buffer posPong{
    vec2 posRead[];
};

layout(binding=2) buffer velPing{
    vec2 velWrite[];
};
layout(binding=3) buffer velPong{
	vec2 velRead[];
};

uniform int W;
uniform int H;
uniform int xMax;
uniform int yMax;

layout(local_size_x = 100, local_size_y = 1, local_size_z = 1) in;

// slime behaviour -------------------------------------------------------------
layout(rgba32f, binding=4) uniform readonly image2D trailMap;

float pi = 3.14159;

uniform float sensorDst;
uniform int sensorSize;
uniform float sensorAngle;
uniform float deltaTime;
uniform float turnSpeed;

uniform float timeLastFrame;
//------------------------------------------------------------------------------

vec2 sensorDir(vec2 myVel, float sensorAngle){

        float xDir = cos(sensorAngle)*myVel.x - sin(sensorAngle)*myVel.y;
        float yDir = sin(sensorAngle)*myVel.x + cos(sensorAngle)*myVel.y;

        float lenght = sqrt(xDir*xDir + yDir*yDir);

        return vec2(xDir/lenght, yDir/lenght);

}

// hash function
uint hash(uint state){
    state ^= 2747636419u;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    state ^= state >> 16;
    state *= 2654435769u;
    return state;
}

float scaleToRange01(uint state){
    return state / 4294967295.0;
}


void main(){

	int idx = int(gl_GlobalInvocationID.x);

	vec2 newPos = posRead[idx] + deltaTime*velRead[idx];
	vec2 newVel = velRead[idx];

    // elastic boundary
	if(newPos.x < 0 || newPos.x > xMax){
		newPos = posRead[idx];
		newVel.x *= -1;
	}
	if(newPos.y < 0 || newPos.y > yMax){
		newPos = posRead[idx];
		newVel.y *= -1;
	}


/*
    // toroidal boundary
    if(newPos.x < 0){
		newPos.x += W;
	}else if(newPos.x > W){
        newPos.x -= W;
    }

    if(newPos.y < 0){
		newPos.y += H;
	}else if(newPos.y > H){
        newPos.y -= H;
    }
*/

//--slime behaviour-------------------------------------------------------------

    // left sensing
    vec2 leftSensorDir = sensorDir(newVel, -sensorAngle);
    vec2 leftSensorPos = newPos + leftSensorDir*sensorDst;
    int sensorCentreX = int(leftSensorPos.x*xMax/W);
    int sensorCentreY = int(leftSensorPos.y*yMax/H);
    float sum = 0;
    for (int offsetX = -sensorSize; offsetX <= sensorSize; offsetX ++) {
        for (int offsetY = -sensorSize; offsetY <= sensorSize; offsetY ++) {

            // boundaries handling
            int sampleX = min(W-1, max(0, sensorCentreX + offsetX));
            int sampleY = min(H-1, max(0, sensorCentreY + offsetY));

            // take red channel as reference
            sum += imageLoad(trailMap, ivec2(sampleX, sampleY)).r;
        }
    }
    float leftWeight = sum;


    // forward sensing
    vec2 forwardSensorDir = sensorDir(newVel, 0);
    vec2 forwardSensorPos = newPos + forwardSensorDir*sensorDst;
    sensorCentreX = int(forwardSensorPos.x*xMax/W);
    sensorCentreY = int(forwardSensorPos.y*yMax/H);
    sum = 0;
    for (int offsetX = -sensorSize; offsetX <= sensorSize; offsetX ++) {
        for (int offsetY = -sensorSize; offsetY <= sensorSize; offsetY ++) {

            // boundaries handling
            int sampleX = min(W-1, max(0, sensorCentreX + offsetX));
            int sampleY = min(H-1, max(0, sensorCentreY + offsetY));

            // take red channel as reference
            sum += imageLoad(trailMap, ivec2(sampleX, sampleY)).r;
        }
    }
    float forwardWeight = sum;


    // right sensing
    vec2 rightSensorDir = sensorDir(newVel, +sensorAngle);
    vec2 rightSensorPos = newPos + rightSensorDir*sensorDst;
    sensorCentreX = int(rightSensorPos.x*xMax/W);
    sensorCentreY = int(rightSensorPos.y*yMax/H);
    sum = 0;
    for (int offsetX = -sensorSize; offsetX <= sensorSize; offsetX ++) {
        for (int offsetY = -sensorSize; offsetY <= sensorSize; offsetY ++) {

            // boundaries handling
            int sampleX = min(W-1, max(0, sensorCentreX + offsetX));
            int sampleY = min(H-1, max(0, sensorCentreY + offsetY));

            // take red channel as reference
            sum += imageLoad(trailMap, ivec2(sampleX, sampleY)).r;
        }
    }
    float rightWeight = sum;

    // a bit of noise
    uint random = hash(uint(newPos.y*W + newPos.x + hash(uint(idx + timeLastFrame * 100000))));
    float noise = scaleToRange01(random);

    float turnAngle = 0;

    // set of rules 1
    if(leftWeight > forwardWeight && leftWeight > rightWeight){
        // turn left
        turnAngle = -sensorAngle*turnSpeed*noise;
    }else if(rightWeight > forwardWeight && rightWeight > leftWeight){
        // turn right
        turnAngle = +sensorAngle*turnSpeed*noise;
    }

/*
    // set of rules 2
    if(forwardWeight > leftWeight && forwardWeight > rightWeight){
        turnAngle += 0;
    }else if(forwardWeight < leftWeight && forwardWeight < rightWeight){
        turnAngle += (noise - 0.5)*2*turnSpeed*deltaTime;
    }else if(leftWeight > rightWeight){
        turnAngle += noise*turnSpeed*deltaTime;
    }else if(rightWeight > leftWeight){
        turnAngle -= noise*turnSpeed*deltaTime;
    }
*/

    newVel.x = cos(turnAngle)*newVel.x - sin(turnAngle)*newVel.y;
    newVel.y = sin(turnAngle)*newVel.x + cos(turnAngle)*newVel.y;

//------------------------------------------------------------------------------

    // low cap for speed
    float absVel = sqrt(newVel.x*newVel.x + newVel.y*newVel.y);
    if(absVel < 1){
        newVel /= absVel;
    }

	posWrite[idx] = newPos;
	velWrite[idx] = newVel;

}
