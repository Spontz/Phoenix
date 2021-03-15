#type vertex
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
}

#type fragment
#version 330 core
out vec4 fragColor;

in vec2 TexCoords;

#define STEPS 128
#define EPS 0.001
#define FAR 50.0

uniform vec2 iResolution;
uniform float iTime;
uniform float beat;

// Esta funcion "renderiza" la escena, y nos dice 
float map (in vec3 pos)
{
    float d1 = length(pos) - 0.25; // Sphere
    
    float d2 = pos.y - (-0.25); // Floor
    
    return min(d1,d2); // Devolvemos lo que encontramos antes
}

vec3 calcNormal (in vec3 pos)
{
	vec2 e = vec2(0.0001, 0.0);
	return normalize( vec3(map(pos+e.xyy)-map(pos-e.xyy),
                           map(pos+e.yxy)-map(pos-e.yxy),
                           map(pos+e.yyx)-map(pos-e.yyx) ) );
}

// Calculamos la distancia el rayo (desde la posicion de la cámara hasta que toca algun objeto)
// si no encuentra nada, t valdrá >= 20.0
float castRay (in vec3 ro, vec3 rd)
{
    float t=0.0;
    for ( int i=0; i<100; i++ )
    {
        vec3 pos = ro + t*rd;	// Calculamos el rayo
        float h = map(pos); // Evaluamos si hemos tocado algun objeto
        if (h<0.001) break; // If we are inside the sphere, there is no needd to evaluate the ray
        t += h;
        if (t>20.0) break; // If we are far away, there is no need to evaluate more the ray
    }
    if (t>20.0) t=-1.0;
    return t;
}

void main()
{
	vec2 fragCoord = TexCoords*iResolution;
	vec2 p = (2.0*fragCoord-iResolution.xy)/iResolution.y;
	
    
    float an = 0.7*iTime;	// Angle
    //float an = 10.0*iMouse.x/iResolution.x;	// Angle
    
    //vec3 ro = vec3 (0.0, 0.0, 1.0);			// Ray Origin (Camera position)
    vec3 ro = vec3 (1.0*sin(an), 0.0, 1.0*cos(an));			// Ray Origin (Camera position)
    
    vec3 ta = vec3 (0.0, 0.0, 0.0); 		// Camera
    vec3 ww = normalize( ta - ro );
    vec3 uu = normalize( cross (ww, vec3 (0,1,0))); // right vector of the camera (with the Up vector)
    vec3 vv = normalize( cross (uu,ww));
    
    //vec3 rd = normalize( vec3 (p, -1.5) );	// Ray Direction (apunta al pixel que estamos pintando)
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );	// Ray Direction (apunta al pixel que estamos pintando)
    
    
    vec3 col = vec3(0.4, 0.75, 1.0) - 0.7*rd.y;	// Sky color, usamos la variable del rayo (rd.y) para hacer un gradiente
    
    col = mix( col, vec3(0.7, 0.75, 0.8), exp(-10.0*rd.y) );
    
    float t=castRay( ro, rd );
    
    if( t>0.0 )
    {
        vec3 pos = ro + t*rd;// Recalculamos el punto en el espacio
        vec3 nor = calcNormal(pos);
        
        vec3 mate = vec3(0.18); // Material
        
        vec3 sun_dir = normalize( vec3(0.8, 0.4, 0.2) );				// Sun direction
        float sun_dif = clamp( dot(nor, sun_dir), 0.0, 1.0); 			// Sun diffuse ammount
        float sun_sha = step( castRay( pos+nor*0.001, sun_dir), 0.0);	// Sun Shadow: evaluamos si el punto a pintar (pos) está oscuro debido al sol
        float sky_dif = clamp( 0.5 + 0.5*dot(nor, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);// Sky diffuse ammount (iluminación del cielo)
        float bou_dif = clamp( 0.5 + 0.5*dot(nor, vec3(0.0,-1.0, 0.0)), 0.0, 1.0);// Bounce diffuse ammount (iluminación del suelo, los reflejos del suelo)
        
		col  = mate*vec3(7.0, 4.5, 3.0)*sun_dif*sun_sha;// Sun diffuse component (values goes arround 10)
        col += mate*vec3(0.5, 0.8, 0.9)*sky_dif;		// Sky diffuse ligthning component (values goes arround 1)
        col += mate*vec3(0.7, 0.3, 0.1)*bou_dif;		// Bounce lightning
        
    }
    
    col = pow ( col, vec3(0.4545) ); // Aplicamos la gamma correction
    
    fragColor = vec4(col,1.0);
}
