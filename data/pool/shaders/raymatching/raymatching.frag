#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

#define STEPS 128
#define EPS 0.001
#define FAR 50.0

uniform sampler2D texture_diffuse1;
uniform float iTime;
uniform float beat;

// OPERATIONS
float smin( float a, float b, float k )
{
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}

float opS( float d1, float d2 )
{
    return max(-d1,d2);
}

vec3 opTwist( vec3 p )
{
    float tw = sin(iTime)*1.0;
    float  c = cos(tw*p.y+1.0);
    float  s = sin(tw*p.y+1.0);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}


// rotate functions
mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}
// Rotate object
vec3 rotate(vec3 v, vec3 axis, float angle) {
	mat4 m = rotationMatrix(axis, angle);
	return (m * vec4(v, 1.0)).xyz;
}

// Rotate camera
mat3 rotX(float ang) {
    float c = cos(ang), s = sin(ang);
    return mat3(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c);
}

mat3 rotY(float ang) {
    float c = cos(ang), s = sin(ang);
    return mat3(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c);
}

mat3 rotZ(float ang) {
    float c = cos(ang), s = sin(ang);
    return mat3(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0);
}


float length8( vec2 p )
{
    p = p*p; p = p*p; p = p*p;
    return pow( p.x + p.y, 1.0/8.0 );
}

///////////////////////////
// OBJECTS
float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}
// Sphere displaced
// p - pos
// s - radius
// y - Displacement factor
float sdSphereDisplaced( vec3 p, float s, float y )
{
  float displacement = sin(y)*sin(5.0 * p.x) * sin(5.0 * p.y) * sin(5.0 * p.z) * 0.25;
  return length(p)-s+displacement;
}

////////////////////////////
// Es la distancia del punt que estem actual al nostre objecte.
vec2 isosurface(vec3 p) {
    float rep = 4.0;

    vec2 t = vec2(1.5, 0.42);
    vec2 t2 = vec2(1.5, 0.28);
    vec3 pos;
    float dis,dis2;


    // sphere 0 - central
    pos = p;
    pos = rotate(p, vec3(1.0,1.0,0.0), iTime*5.0);
    dis=sdSphereDisplaced(pos,1.0+beat*2.0, iTime*2.0);

    // sphere 1
    pos = p;
    pos.x -= 3.0*sin(iTime);
    pos.y += 1.0*cos(iTime);
    dis2=sdSphere(pos,1.1);   
    // Calculate distance
    dis = smin(dis,dis2,5.0);
    
    // sphere 3
    pos = p;
    pos.z -= 2.0*sin(iTime);
    pos.y += 0.5+3.0*sin(iTime)*cos(iTime);
    pos.z += 0.2+2.0*cos(iTime)*cos(iTime);
    pos = rotate(pos, vec3(1.0,1.0,1.0), iTime*4.0);
    dis2=sdSphereDisplaced(pos,1.3, iTime*2.0);
    // Calculate distance
    dis = smin(dis,dis2,7.0);

    // sphere 4
    pos = p;
    pos.x += 1.0*cos(iTime);
    pos.y += 0.5+4.0*sin(iTime)*sin(iTime);
    pos.z += 1.0*cos(iTime);
    dis2=sdSphere(pos,0.8);
    // Calculate distance
    dis = smin(dis,dis2,5.0);

    // sphere 5
    pos = p;
    pos.x += 0.4+1.0*sin(iTime);
    pos.z -= 4.0*sin(iTime)*cos(iTime);
    dis2=sdSphere(pos,1.6);
    // Calculate distance
    dis = smin(dis,dis2,5.0);
    
/*    // sphere 6 SUBSTRACTION!!!
    pos = p;
    pos.x += 0.2+1.0*cos(iTime);
    pos.z += 2.0*cos(iTime)*cos(iTime);
    dis2=sdSphere(pos,1.6);
    // Calculate distance
    dis = opS(dis2,dis);
*/    
    return vec2(dis,10.0);
}


vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3( 0.001, 0.0, 0.0 );
    vec3 nor = vec3(
        isosurface(pos+eps.xyy).x - isosurface(pos-eps.xyy).x,
        isosurface(pos+eps.yxy).x - isosurface(pos-eps.yxy).x,
        isosurface(pos+eps.yyx).x - isosurface(pos-eps.yyx).x );

    return normalize(nor);
}


vec3 shade(vec3 ro, vec3 rd, float t, float m) {
    //vec3 n = normalize(calcNormal(ro +t*rd));
    //return vec3(1.0)*dot(-rd, n);
    vec3 nor = normalize(calcNormal(ro +t*rd));
    vec3 ref = reflect( rd, nor );


    vec3  lig = normalize( vec3(sin(iTime*2.0)*1.5, cos(iTime*2.0)*0.7, -0.5) );
    float amb = clamp( 0.5+0.5, 0.0, 1.0 );
    float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
    float spe = pow(clamp( dot( ref, lig ), 0.0, 1.0 ),16.0);

    vec3 brdf = vec3(0.0);
    brdf += 1.20*dif*vec3(1.00,0.90,0.60);
    brdf += 2.20*spe*vec3(1.00,0.90,0.60)*dif;
    brdf += amb*vec3(0.50,0.70,1.00);

    vec3 col = 0.45 + 0.3* vec3(0.05+m/10.0,0.08,0.10);

    col = col*brdf;

    col += 0.5*pow(clamp(0.9+dot(nor, rd), 0.0, 1.0), 3.0);

    //col*= (texture(texture_diffuse1, (ro +t*rd).xy+vec2(0.1,0.1)).r)*1.6;
    col*= (texture(texture_diffuse1, (ro +t*rd).xz).rgb)*1.5;
    
    // See normals
    //col = nor;
    return col;
}



mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
    vec3 cw = normalize(ta-ro);
    vec3 cp = vec3(sin(cr), cos(cr),0.0);
    vec3 cu = normalize( cross(cw,cp) );
    vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}

void main( void )
{
	vec2 uv = TexCoords.xy-0.5;

	mat3 m = rotX(2.0*sin(1.0));
    m*=rotZ(4.0*sin(1.0));
    m*=rotY(3.0*sin(1.0));

    vec3 ro =  m*vec3(0.0, 0.0,10.0);
    vec3 rd =  m*normalize(vec3( uv, -1.0)); 

    float t=0.0;
    vec2 d= vec2(0.01,0);

    for (int i=0; i < STEPS; ++i) {

        d = isosurface(ro + (t*rd));
        if (d.x < EPS || d.x > FAR) break;
        t += d.x;

    }

    vec3 col = d.x< EPS ? shade( ro, rd, t, d.y) :  vec3(0.0);
    FragColor = vec4(col ,1.0);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}