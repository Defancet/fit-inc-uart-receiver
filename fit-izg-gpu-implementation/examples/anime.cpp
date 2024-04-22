/*!
 * @file
 * @brief This file contains implementation of czech flag rendering method
 *
 * @author Tomáš Milet, imilet@fit.vutbr.cz
 */

#include <framework/programContext.hpp>
#include <vector>

namespace animeMethod{

/**
 * @brief Czech flag rendering method
 */
class Method: public ::Method{
  public:
    Method(MethodConstructionData const*);
    virtual ~Method(){}
    virtual void onDraw(Frame&frame,SceneParam const&sceneParam) override;
    virtual void onUpdate(float dt) override;
    CommandBuffer           commandBuffer       ;///< command buffer
    GPUMemory               mem                 ;///< gpu memory
    float                   time = 0.f          ;///< elapsed time
};

#define SQRT3 1.732050807

// 2D SDF functions from iq: https://iquilezles.org/articles/distfunctions2d

float dot2(glm::vec2 v ) { return glm::dot(v,v); }
float dot3(glm::vec2 v ) { return glm::dot(v,v) * glm::length(v); }
float dot4(glm::vec2 v ) { return glm::dot(v,v) * glm::dot(v,v); }

float cross2(glm::vec2 a, glm::vec2 b ) { return a.x*b.y - a.y*b.x; }

float sdfCircle(float r, glm::vec2 p )
{
    return glm::length(p) - r;
}

float sdEgg(float ra, float rb, glm::vec2 p)
{
    const float k = glm::sqrt(3.0);
    p.x = glm::abs(p.x);
    float r = ra - rb;
    return ((p.y<0.0)       ? glm::length(p) - r :
            (k*(p.x+r)<p.y) ? glm::length(p - glm::vec2(0,k*r)) :
                              glm::length(glm::vec2(p.x+r,p.y    )) - 2.0*r) - rb;
}

float sdParabola(float k, glm::vec2 pos)
{
    pos.x = glm::abs(pos.x);
    float ik = 1.0/k;
    float p = ik*(pos.y - 0.5*ik)/3.0;
    float q = 0.25*ik*ik*pos.x;
    float h = q*q - p*p*p;
    float r = glm::sqrt(glm::abs(h));
    float x = (h>0.0) ? 
        glm::pow(q+r,1.0/3.0) - glm::pow(abs(q-r),1.0/3.0)*sign(r-q) :
        2.0*cos(atan(r,q)/3.0)*sqrt(p);
    return length(pos-glm::vec2(x,k*x*x)) * sign(pos.x-x);
}


// unsigned distance to a quadratic bezier
float udBezier(in glm::vec2 A, in glm::vec2 B, in glm::vec2 C, in glm::vec2 pos)
{    
    glm::vec2 a = B - A;
    glm::vec2 b = A - 2.0*B + C;
    glm::vec2 c = a * 2.0;
    glm::vec2 d = A - pos;

    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b))/3.0;
    float kz = kk * dot(d,a);      

    float res = 0.0;

    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx - 3.0*ky) + kz;
    float h = q*q + 4.0*p3;

    if( h>=0.0 ) 
    {   // 1 root
        h = sqrt(h);
        glm::vec2 x = (glm::vec2(h,-h)-q)/2.0;
        glm::vec2 uv = sign(x)*glm::pow(abs(x), glm::vec2(1.0/3.0));
        float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );
        res = dot2(d+(c+b*t)*t);
    }
    else 
    {   // 3 roots
        float z = sqrt(-p);
        float v = acos(q/(p*z*2.0))/3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        glm::vec3  t = clamp( glm::vec3(m+m,-n-m,n-m)*z-kx, 0.0, 1.0 );
        res = min( dot2(d+(c+b*t.x)*t.x),
                   dot2(d+(c+b*t.y)*t.y) );
        // the third root cannot be the closest. See https://www.shadertoy.com/view/4dsfRS
        // res = min(res,dot2(d+(c+b*t.z)*t.z));
    }
    
    return sqrt( res );
}

// signed distance to a quadratic bezier
float sdBezier(in glm::vec2 A, in glm::vec2 B, in glm::vec2 C, in glm::vec2 pos)
{    
    glm::vec2 a = B - A;
    glm::vec2 b = A - 2.0*B + C;
    glm::vec2 c = a * 2.0;
    glm::vec2 d = A - pos;

    float kk = 1.0/dot(b,b);
    float kx = kk * dot(a,b);
    float ky = kk * (2.0*dot(a,a)+dot(d,b))/3.0;
    float kz = kk * dot(d,a);      

    float res = 0.0;
    float sgn = 0.0;

    float p = ky - kx*kx;
    float p3 = p*p*p;
    float q = kx*(2.0*kx*kx - 3.0*ky) + kz;
    float h = q*q + 4.0*p3;

    if( h>=0.0 ) 
    {   // 1 root
        h = sqrt(h);
        glm::vec2 x = (glm::vec2(h,-h)-q)/2.0;
        glm::vec2 uv = sign(x)*glm::pow(abs(x), glm::vec2(1.0/3.0));
        float t = clamp( uv.x+uv.y-kx, 0.0, 1.0 );
        glm::vec2  q = d+(c+b*t)*t;
        res = dot2(q);
    	sgn = cross2(c+2.0*b*t,q);
    }
    else 
    {   // 3 roots
        float z = sqrt(-p);
        float v = acos(q/(p*z*2.0))/3.0;
        float m = cos(v);
        float n = sin(v)*1.732050808;
        glm::vec3  t = clamp( glm::vec3(m+m,-n-m,n-m)*z-kx, 0.0, 1.0 );
        glm::vec2  qx=d+(c+b*t.x)*t.x; float dx=dot2(qx), sx = cross2(c+2.0*b*t.x,qx);
        glm::vec2  qy=d+(c+b*t.y)*t.y; float dy=dot2(qy), sy = cross2(c+2.0*b*t.y,qy);
        if( dx<dy ) { res=dx; sgn=sx; } else {res=dy; sgn=sy; }
    }
    
    return sqrt( res )*sign(sgn);
}


float sdEllipse(in glm::vec2 ab,  in glm::vec2 p)
{
    p = abs(p); if( p.x > p.y ) {p=p.yx;ab=ab.yx;}
    float l = ab.y*ab.y - ab.x*ab.x;
    float m = ab.x*p.x/l;      float m2 = m*m; 
    float n = ab.y*p.y/l;      float n2 = n*n; 
    float c = (m2+n2-1.0)/3.0; float c3 = c*c*c;
    float q = c3 + m2*n2*2.0;
    float d = c3 + m2*n2;
    float g = m + m*n2;
    float co;
    if( d<0.0 )
    {
        float h = acos(q/c3)/3.0;
        float s = cos(h);
        float t = sin(h)*sqrt(3.0);
        float rx = sqrt( -c*(s + t + 2.0) + m2 );
        float ry = sqrt( -c*(s - t + 2.0) + m2 );
        co = (ry+sign(l)*rx+abs(g)/(rx*ry)- m)/2.0;
    }
    else
    {
        float h = 2.0*m*n*sqrt( d );
        float s = sign(q+h)*glm::pow(abs(q+h), 1.0/3.0);
        float u = sign(q-h)*glm::pow(abs(q-h), 1.0/3.0);
        float rx = -s - u - c*4.0 + 2.0*m2;
        float ry = (s - u)*sqrt(3.0);
        float rm = sqrt( rx*rx + ry*ry );
        co = (ry/sqrt(rm-rx)+2.0*g/rm-m)/2.0;
    }
    glm::vec2 r = ab * glm::vec2(co, sqrt(1.0-co*co));
    return length(r-p) * sign(p.y-r.y);
}

// uneven capsule
float sdUnevenCapsuleY( in glm::vec2 p, in float ra, in float rb, in float h )
{
    p.y += h;
	p.x = abs(p.x);
    
    float b = (ra-rb)/h;
    glm::vec2  c = glm::vec2(sqrt(1.0-b*b),b);
    float k = cross2(c,p);
    float m = dot(c,p);
    float n = dot(p,p);
    
         if( k < 0.0   ) return sqrt(n)               - ra;
    else if( k > c.x*h ) return sqrt(n+h*h-2.0*h*p.y) - rb;
                         return m                     - ra;
}

glm::vec2 opCheapBend(in glm::vec2 p, float k)
{
    float c = cos(k*p.x);
    float s = sin(k*p.x);
    mat2  m = mat2(c,s,-s,c);
    return m*p;
}

float disp(glm::vec2 p, float f)
{
    float d = 0.0;
    for (int i = 0; i <4; ++i)
    {
 		d += sin(f*p.x)*sin(f*p.y);
        p *= 1.9;
        d *= 2.0;
    }
    return d / 16.0;
}

float sdfLine(glm::vec2 p0, glm::vec2 p1, float width, glm::vec2 coord)
{
    glm::vec2 dir0 = p1 - p0;
	glm::vec2 dir1 = coord - p0;
	float h = clamp(dot(dir0, dir1)/dot(dir0, dir0), 0.0, 1.0);
	return (length(dir1 - dir0 * h) - width * 0.5);
}

float sdfTriangleDist(float width, float height, glm::vec2 p)
{
    glm::vec2 q = glm::vec2(width, height);
    p.x = abs(p.x);
    glm::vec2 a = p - q*clamp( dot(p,q)/dot(q,q), 0.0, 1.0 );
    glm::vec2 b = p - q*glm::vec2( clamp( p.x/q.x, 0.0, 1.0 ), 1.0 );
    float s = -sign( q.y );
    glm::vec2 d = min( glm::vec2( dot(a,a), s*(p.x*q.y-p.y*q.x) ),
                  glm::vec2( dot(b,b), s*(p.y-q.y)  ));
    return -sqrt(d.x)*sign(d.y);
}

float sdTriangle(in glm::vec2 p0, in glm::vec2 p1, in glm::vec2 p2, in glm::vec2 p)
{
    glm::vec2 e0 = p1-p0, e1 = p2-p1, e2 = p0-p2;
    glm::vec2 v0 = p -p0, v1 = p -p1, v2 = p -p2;
    glm::vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
    glm::vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
    glm::vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
    float s = sign( e0.x*e2.y - e0.y*e2.x );
    glm::vec2 d = min(min(glm::vec2(dot(pq0,pq0), s*(v0.x*e0.y-v0.y*e0.x)),
                     glm::vec2(dot(pq1,pq1), s*(v1.x*e1.y-v1.y*e1.x))),
                     glm::vec2(dot(pq2,pq2), s*(v2.x*e2.y-v2.y*e2.x)));
    return -sqrt(d.x)*sign(d.y);
}

float sdTrapezoid(in float r1, float r2, float he, in glm::vec2 p)
{
    glm::vec2 k1 = glm::vec2(r2,he);
    glm::vec2 k2 = glm::vec2(r2-r1,2.0*he);
    p.x = abs(p.x);
    glm::vec2 ca = glm::vec2(p.x-min(p.x,(p.y<0.0)?r1:r2), abs(p.y)-he);
    glm::vec2 cb = p - k1 + k2*clamp( dot(k1-p,k2)/dot2(k2), 0.0, 1.0 );
    float s = (cb.x<0.0 && ca.y<0.0) ? -1.0 : 1.0;
    return s*sqrt( min(dot2(ca),dot2(cb)) );
}


float sdfUnion( const float a, const float b )
{
    return min(a, b);
}

float smoothUnion(float d1, float d2, float k)
{
    float h = clamp(0.5 + 0.5*(d2 - d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k * h * (1.0-h);
}

float sdfDifference( const float a, const float b)
{
    return max(a, -b);
}

float sdfIntersection( const float a, const float b )
{
    return max(a, b);
}

float opSmoothIntersection( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) + k*h*(1.0-h); }

float opSmoothSubtraction( float d1, float d2, float k ) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); }

glm::vec2 rotate(glm::vec2 uv, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    mat2 m = mat2(c,s,-s,c);
    return m * uv;
}

glm::vec3 shadeDistance(float d) {
    d *= .5;
    float dist = d*120.;
    float banding = max(sin(dist), 0.0);
    float strength = sqrt(1.-exp(-abs(d)*2.));
    float pattern = mix(strength, banding, (0.6-abs(strength-0.5))*0.3);
    
    glm::vec3 color = glm::vec3(pattern);
    
    color *= d > 0.0 ? glm::vec3(1.0,0.56,0.4) : glm::vec3(0.4,0.9,1.0);

    return color;
}
glm::vec2 hash( glm::vec2 x )  // replace this by something better
{
    const glm::vec2 k = glm::vec2( 0.3183099, 0.3678794 );
    x = x*k + k.yx;
    return -1.0 + 2.0*fract( 16.0 * k*fract( x.x*x.y*(x.x+x.y)) );
}

float noise( in glm::vec2 p )
{
    glm::vec2 i = floor( p );
    glm::vec2 f = fract( p );
	
	glm::vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( hash( i + glm::vec2(0.0,0.0) ), f - glm::vec2(0.0,0.0) ), 
                     dot( hash( i + glm::vec2(1.0,0.0) ), f - glm::vec2(1.0,0.0) ), u.x),
                mix( dot( hash( i + glm::vec2(0.0,1.0) ), f - glm::vec2(0.0,1.0) ), 
                     dot( hash( i + glm::vec2(1.0,1.0) ), f - glm::vec2(1.0,1.0) ), u.x), u.y);
}

float fwidth2(float d){
  return .004f;
}

glm::vec4 render(float d, glm::vec3 color, float w)
{
    float anti = fwidth2(d) * w;
    return glm::vec4(color, glm::smoothstep(anti, -anti, d));
}

glm::vec4 render(float d, glm::vec4 color)
{
    float anti = fwidth2(d) * 1.0;
    return glm::vec4(glm::vec3(color), color.a * glm::smoothstep(anti, -anti, d));
}

glm::vec4 render_stroked(float d, glm::vec3 color, float stroke)
{
    float anti = fwidth2(d) * 1.0;
    glm::vec4 strokeLayer = glm::vec4(glm::vec3(0.01), glm::smoothstep(anti, -anti, d - stroke));
    glm::vec4 colorLayer = glm::vec4(color, glm::smoothstep(anti, -anti, d));
    return glm::vec4(mix(glm::vec3(strokeLayer), glm::vec3(colorLayer), colorLayer.a), strokeLayer.a);
}

glm::vec4 render_stroked_masked(float d, glm::vec3 color, float stroke, float stroke_mask)
{
    float anti = fwidth2(d) * 1.0;
    glm::vec4 strokeLayer = glm::vec4(glm::vec3(0.01), glm::smoothstep(anti, -anti, d));
    float se = glm::smoothstep(anti, -anti, stroke_mask);
    glm::vec4 colorLayer = glm::vec4(color, glm::smoothstep(anti, -anti, d + stroke));
    return glm::vec4(mix(mix(glm::vec3(strokeLayer), glm::vec3(colorLayer),  se), glm::vec3(colorLayer), colorLayer.a), strokeLayer.a);
}

void render_layer(glm::vec4&c, glm::vec4 layer)
{ 
    c = glm::vec4(mix(glm::vec3(c), glm::vec3(layer), layer.a),c.a);
}

void render_layer_mul(glm::vec4&c, glm::vec4 layer)
{ 
    c = glm::vec4(mix(glm::vec3(c), glm::vec3(c) * glm::vec3(layer), layer.a),c.a);
}

float exact_intersection(float d1, float d2)
{
    float dmin = glm::min(d1, d2);
    float dmax = glm::max(d1, d2);
    return dmin < 0. ? dmax : dmin;
}

glm::vec4 sdEye(glm::vec2 p)
{
    p += glm::vec2(0.52, -0.15);
    glm::vec2 plt = glm::vec2(-0.035, -0.045); glm::vec2 pmt = glm::vec2(0.56, 0.31); glm::vec2 prt = glm::vec2(0.862, 0.0421);
    glm::vec2 plm = glm::vec2(0.039, -0.3);                                glm::vec2 prm = glm::vec2(0.87, -0.088);
    glm::vec2 plb = glm::vec2(0.31, -0.35); glm::vec2 pmb = glm::vec2(0.55, -0.35); glm::vec2 prb = glm::vec2(0.84, -0.25);
    
    float d1 = sdBezier(prt, pmt, plt, p);
    float d2 = sdBezier(plt, plm, plb, p);
    float d3 = sdBezier(plb, pmb, prb, p);
    float d4 = sdBezier(prb, prm, prt, p);
    
    float d14 = exact_intersection(d1, d4);
    float d23 = exact_intersection(d2, d3);
    float dd = exact_intersection(d23, d14);
    return glm::vec4(dd, d1, d23, d2);
}


float sdPupil(glm::vec2 p, float r, glm::vec2 offset)
{
    return sdEllipse(glm::vec2(0.2, 0.3) * r, p - offset);
}

float glm::pow2(float x) { return x * x; }

glm::vec4 sdHair(glm::vec2 p)
{
    float def1 = p.y + 1.5;
    float def2 = p.y + 0.3;
    
    p.x += glm::pow2(3.8 * max(0.1 - def1 * def1 * 0.15, 0.)) * sign(p.x) * glm::smoothstep(0.40, 0.43, abs(p.x));
    p.x -= glm::pow2(2.5 * max(0.1 - def1 * def1 * 0.15, 0.)) * float(p.x < 0.);
    p.x += glm::pow2(1.5 * max(0.1 - def2 * def2 * 0.35, 0.)) * float(p.x < -0.46);
    
	float d = sdUnevenCapsuleY( p, 0.73, 0.51, 2.1 );
    
    float dcut = 0.16 -p.y - p.x * 0.05;
    float dsub = abs(p.x) - 0.41;
    dsub = max(dsub, -dcut);
    float d3 = max(d, -dsub);
    return glm::vec4(d3, d, dcut, dsub);
}

const float f[]   = float[](0.4, 0.23, 0.1, 0.05);
const float off[] = float[](-0.4, 0.1, 0.5, 0.5);
const float amp[] = float[](8.0, 12.4, 22.4, 32.4);
const float bias[] = float[](0.4, 0.2, 0.2, 0.2);
const int n = 4;
glm::vec4 sdHairBands(glm::vec2 p)   
{	
    glm::vec4 dh = sdHair(p);
    float d = dh.x;
    float dcut = 0.16 -p.y - p.x * 0.05;
    
    p += 0.2 * sin(p.x / 0.4) * (1.0-cos((p.y + 0.15) / 0.4));
    
    float db = -(0.2 - abs(dcut));
    float dmod = -0.2;
    float b1 = 0.0;
    
    for (int i = 0; i< n; ++i)
    {
    	b1 = -amp[i] * (max(mod(p.x + off[i], f[i]), f[i] / 2. ) -5./4.*f[i] + max(mod(-p.x - off[i], f[i]), f[i]/2. ));
    	b1 = -glm::pow(max(b1, 0.0), 2.5) + bias[i];
    	dmod = max(dmod, -b1);
    }

    float k = 4.0;
    dmod = tanh(k * dmod) / k;
    db += dmod;
    
    db += 0.1 * max(1.0 - 5.0 * p.x * p.x, 0.) - 0.1;
    db *= 0.5;
    db = max(db, dh.y);
    
    d = min(db, max(d, -0.1));
        
    return glm::vec4(d, dh.y, dh.z, dh.w);
}
 
#define LayerF(d, color) render_layer(fragColor, render(d, color, 1.))
#define LayerFM(d, color) render_layer_mul(fragColor, render(d, color, 1.))
#define LayerFMW(d, color, w) render_layer_mul(fragColor, render(d, color, w))
#define LayerS(d, color, stroke) render_layer(fragColor, render_stroked(d, color, stroke))
#define LayerSM(d, color, stroke, mask) render_layer(fragColor, render_stroked_masked(d, color, stroke, mask))

struct Params
{
    float size;
    float pixSize;
    float yaw;
    float lj;
    float wj;
    float wc;
    float th;
    float trh;
    float radius;
    float m;
    float stroke;
};

float make_head(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    float a = sdfTriangleDist(p.wj, p.th, (uv + glm::vec2(0.0, p.lj)));
    float b = sdTrapezoid(p.wj, p.wc, p.trh, (uv + glm::vec2(0.0, p.lj - p.th - p.trh)));
    float c = sdfCircle(p.wc * 1.005, uv - glm::vec2(0.0, p.m));
    c = sdfIntersection(c, -uv.y + p.m);

    float e = sdEgg(p.wc * 1.005, 0.04, (uv - glm::vec2(0.0, p.m))  * glm::vec2(1.0, -1.0)) ;
    
    float d = 1e6;
    d = sdfUnion(a, b);
    d = sdfUnion(d, c);
    d = mix(d, e, 0.4 * glm::smoothstep(0.1, p.wj, abs(uv.x)));
    
    d -= p.radius;
    d += disp(uv, 20.0) * 0.001;  
    
    LayerS(d, glm::vec3(0.757, 0.772, 0.796), p.stroke);
    return d;
}


void make_hair_back(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv = rotate(uv, -0.09);
    uv += glm::vec2(0.02, -0.31);
    uv *= 1.35;
    float d = sdHair(uv * glm::vec2(1.2, 1.0) + glm::vec2(0.01, 0.12)).y;

    LayerF(d, glm::vec3(0.54, 0.37, 0.46));
}
  
void make_hair_shadow(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv = rotate(uv, -0.09);
    uv += glm::vec2(0.02, -0.31);
    uv *= 1.35;
    float ds = sdHairBands(uv * glm::vec2(1.02, 1.0) + glm::vec2(-0.03, 0.05)).x;
    LayerFMW(ds, glm::vec3(0.752, 0.66, 0.69) * 0.9, 3.0);
}

void make_hair(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv = rotate(uv, -0.09);
    uv += glm::vec2(0.02, -0.31);
    uv *= 1.35;
    float def = max(-uv.x + 0.6 * uv.y - 0.45, 0.) * 2.0;
    uv.x -= def * def;
    float d = sdHairBands(uv).x;

    d += disp(uv, 2.0) * 0.005;  
    d += disp(uv, 20.0) * 0.001;  

    LayerS(d, glm::vec3(0.75, 0.67, 0.76), p.stroke * 1.5);
    LayerF(d + 0.18, glm::vec3(0.75, 0.84, 0.87));
}

void _make_hair2(inout glm::vec4 fragColor, Params p, glm::vec2 uv, float l)
{
    uv *= 1.35;
    float def1 = uv.y + 1.5;
    uv.x -= glm::pow(2.5 * max(0.1 - def1 * def1 * 0.15, 0.), 2.0) * (1.0 - 0.6 * l);
    uv /= 1.35;
    float d = abs(0.33 - 0.02 * l + uv.x) - 0.03 * (1.0 + 0.5 * (1. - l ) + uv.y * (0.4 + 0.3 * (1.-l)));
    
    d = max(d, uv.y);
    
    float m = -(uv.y - 0.14 + l * 0.07 - uv.x * 0.6);
    
    m = mix(m, min(m, (uv.x + 0.33 - 0.02 * l)), uv.y < -0.75);

    d += disp(uv, 2.0) * 0.005;  
    d += disp(uv, 20.0) * 0.001;  

    LayerSM(d, glm::vec3(0.75, 0.67, 0.76), p.stroke * 0.5, m);
}

void make_hair2(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    float l = float(uv.x > 0.);
    uv += glm::vec2(0.045, -0.31);
    uv = rotate(uv, -0.12 + 0.025 * l);
    uv.x = -abs(uv.x);
    // uv = mix(uv, uv * glm::vec2(1.03, 0.99) - glm::vec2(0.03, -0.005), l);
    _make_hair2(fragColor, p, uv, l);
}

void make_band_and_horns(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv = rotate(uv, 0.06);
    uv += glm::vec2(0.05, -0.30);
	float d = sdUnevenCapsuleY(uv, 0.73, 0.32, 2.1 );
    uv += glm::vec2(-0.0, 0.09);
	float d2 = sdUnevenCapsuleY(uv, 1.0, 0.335, 2.1 );
    d = max(d, -d2);
    
    float r = float(uv.x > 0.);
    float ir = 1.0 - r;
    uv.x = -abs(uv.x);

    float a = sdfTriangleDist(0.05, 0.08 - 0.03 * r, rotate( uv + glm::vec2(0.24, -0.16 - 0.038 * r), -0.4));
    a = max(a, -uv.x + uv.y - 0.5);
    d = min(d, a);

    LayerS(d, glm::vec3(0.72, 0.79, 0.88), p.stroke);
    
    glm::vec2 p0 = glm::vec2(-0.010, 0.07 - 0.02 * ir);
	float c = max(uv.y - 0.36, 0.) * (r * 0.5 + 0.5);
    uv -= glm::vec2(c * c * 1.25, 0.0);
    float dh = sdTriangle(p0, p0 + glm::vec2(-0.05 - 0.01 * ir, 0.3), p0 + glm::vec2(0.03 * r, 0.13), uv + glm::vec2(0.24, -0.16));
    // dh = max(dh, -uv.x + uv.y - 0.5);
    dh -= 0.005;

    float dhs = max(dh, uv.x + uv.y * 0.5 + 0.07);
    float dhh = max(dh, uv.x + uv.y * 0.18 + 0.195 + 0.012 * ir) + 0.004;
    
    LayerS(dh, glm::vec3(0.52, 0.24, 0.34), p.stroke);
    LayerF(dhs, glm::vec3(0.28, 0.17, 0.27));
    LayerF(dhh, glm::vec3(0.69, 0.62, 0.72));
}

void make_neck(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv += glm::vec2(0.0, p.lj);
    uv += glm::vec2(0.007, 0.1);
    uv *= 2.6;

    glm::vec2 plt = glm::vec2(-0.5,  0.34); glm::vec2 prt = glm::vec2(0.5,  0.34);
    glm::vec2 plm = glm::vec2(-0.4, -0.0);  glm::vec2 prm = glm::vec2(0.4, -0.1);
    glm::vec2 plb = glm::vec2(-0.45, -0.35); glm::vec2 prb = glm::vec2(0.52, -0.2);
    
    float d2 = sdBezier(plt, plm, plb, uv);
    float d4 = sdBezier(prb, prm, prt, uv);
    
    float d24 = exact_intersection(d2, d4);
    float d_caps = max(uv.y - 0.35, -0.34 - uv.y);
    float dd = max(d24, d_caps);
    
    LayerS(dd, glm::vec3(0.54, 0.46, 0.46), p.stroke * 2.6);
}

void make_mouth(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    float mm = sdEllipse(glm::vec2(p.wj * 0.55, p.th * 0.13), opCheapBend(uv + p.yaw * glm::vec2(0.005, 0.), 2.0) + glm::vec2(0.0, p.lj - p.th - 0.01));
    float mme = sdfCircle(0.02, uv + p.yaw * glm::vec2(0.005, 0.) + glm::vec2(-0.03, p.lj - p.th + 0.01));
    mm += disp(uv, 20.0) * 0.002; 
    LayerSM(mm, glm::vec3(0.70, 0.58, 0.6),  p.stroke * (1. + 2. * glm::smoothstep(p.wj * 0.1, p.wj * 0.4, uv.x)), mme);
}

void make_nose(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv.x -= 0.005;
    float nn1 = udBezier(glm::vec2(0.01, p.m - p.th * 1.48), glm::vec2(0.008, p.m - p.th * 1.38), glm::vec2(0.017, p.m - p.th * 1.08), uv);
    float nn2 = udBezier(glm::vec2(0.007, p.m - p.th * 0.62), glm::vec2(0.007, p.m - p.th * 0.75), glm::vec2(0.017, p.m - p.th * 0.87), uv);
    float nn3 = udBezier(glm::vec2(0.01, p.m - p.th * 0.2), glm::vec2(0.017, p.m + p.th * 0.1), glm::vec2(0.04, p.m + p.th * 0.3), uv);
    nn1 = min(nn1, nn2);
    nn1 = min(nn1, nn3);
    
    glm::vec2 p0 = glm::vec2(0.001, p.m - p.th * 0.62);
	float c = max(uv.y - 0.36, 0.);
    float def = dot(glm::vec2(p.th * 0.25, 0.01), p0 - uv) - 0.0003;
    def *= 1200.0;
    def = max(1.0 - def * def, 0.);
    uv.y -= def * 0.015;
    float dh = sdTriangle(p0, p0 + glm::vec2(-0.021, -p.th * 0.4), p0 + glm::vec2(0.01, -p.th * 0.25), uv);

    dh -= 0.008;

    LayerFMW(dh, glm::vec3(0.752, 0.66, 0.69), 1.5);
    LayerS(nn1, glm::vec3(0.), p.pixSize * 0.6);
}

void make_eye(inout glm::vec4 fragColor, Params p, glm::vec2 uv, float l)
{
    uv.x += p.wc * 0.8;
    uv.y -= p.m;
    uv += glm::vec2(-0.05, 0.06);
    uv *= 5.2;
    glm::vec4 d = sdEye(uv);
    LayerF(d.x, glm::vec3(0.73, 0.78, 0.83));
    float w = (1.0 - (uv.x * uv.x / 0.5)) * 1.3;
    LayerS(abs(d.y - 0.03 * w), glm::vec3(0.08, 0.14, 0.18),  0.04 * w);
    
    float dh = max(d.w - 0.13 * (1.45 + 3.2 * uv.y), -d.w);
    dh = max(dh, uv.y - uv.x * 0.9 - 0.65);
    dh = max(dh, -uv.y + uv.x * 0.44 + 0.0);
    LayerF(dh, glm::vec3(0.59, 0.31, 0.44));
    
    float m = uv.y + 0.08;
    LayerF(max(abs(d.z)- 0.015, m), glm::vec3(0.08, 0.14, 0.18));
    
    float p1 = sdPupil(uv, 1.0, glm::vec2(-0.05, 0.) * l);
    float p2 = sdPupil(uv, 0.52, glm::vec2(-0.05, 0.) * l + glm::vec2(0.03, 0.) * (l - 0.5) + glm::vec2(0.0, 0.03));
    float p3 = sdPupil(uv, 0.15, glm::vec2(-0.05, 0.) * l + glm::vec2(0.05, 0.) * (l - 0.5) + glm::vec2(0.0, 0.03));
    float s1 = sdPupil(uv, 0.3, glm::vec2(-0.05, 0.) * l + glm::vec2(0.36, 0.) * (l - 0.5));
    
    LayerS(max(p1, d.x), glm::vec3(0.26, 0.45, 0.45),  p.stroke * 5.2);
    LayerS(max(p2, d.x), glm::vec3(0.26, 0.45, 0.45),  p.stroke * 5.2);
    LayerS(max(p3, d.x), glm::vec3(0.08, 0.14, 0.18),  p.stroke * 5.2);
    LayerF(max(s1, d.x), glm::vec3(0.75, 0.84, 0.87));
    {
    	glm::vec2 _uv = uv + glm::vec2(0.4, -0.38) + glm::vec2(0.15, 0.02) * l - 0.1 * uv.x * l;
    	glm::vec2 plt = glm::vec2(0.1, 0.04); glm::vec2 pmt = glm::vec2(0.5, 0.2); glm::vec2 prt = glm::vec2(0.88, 0.05);
    	float du = udBezier(prt, pmt, plt, _uv);
    	LayerS(du, glm::vec3(0.),  p.stroke * 3.);
    }
}

void make_eyebrow(inout glm::vec4 fragColor, Params p, glm::vec2 uv, float l)
{
    uv.x += p.wc * 0.8;
    uv.y -= p.m;
    uv += glm::vec2(-0.05, 0.06);
    uv *= 5.2;

  	glm::vec2 _uv = uv + glm::vec2(0.4, -0.60) + glm::vec2(0.15, 0.02) * l - 0.1 * uv.x * l;
   	glm::vec2 plt = glm::vec2(-0.01, -0.05); glm::vec2 pmt = glm::vec2(0.5, 0.02); glm::vec2 prt = glm::vec2(0.98, 0.18);
   	float du = udBezier(prt, pmt, plt, _uv) - 1.0 * min(dot3(_uv - glm::vec2(-0.01, -0.05)) * dot2(_uv - glm::vec2(0.98, 0.18)), 0.1);
   	LayerS(du, glm::vec3(0.75, 0.67, 0.76),  p.stroke * 3.);
}

void make_eyes(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    float l = float(uv.x > 0.);
    uv.x = -abs(uv.x);
    uv = mix(uv, uv * glm::vec2(1.03, 0.99) - glm::vec2(0.03, -0.005), l);
    make_eye(fragColor, p, uv, l);
}

void make_eyebrows(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    float l = float(uv.x > 0.);
    uv.x = -abs(uv.x);
    uv = mix(uv, uv * glm::vec2(1.03, 0.99) - glm::vec2(0.03, -0.005), l);
    make_eyebrow(fragColor, p, uv, l);
}

void make_ear(inout glm::vec4 fragColor, Params p, glm::vec2 uv, float l, float headd)
{
    uv += glm::vec2(0.38, 0.065) + glm::vec2(-0.075, 0.02) *l;
    uv = rotate(uv, -0.4);
    float def1 = uv.x - uv.y + 0.0;
    float def2 = uv.x + uv.y * 0.7 - 0.05;
    float def3 = uv.x - uv.y + 0.0;
    uv.x -= max(0.02 - def1 * def1, 0.);
    uv.x -= glm::pow(max(0.1 - def2 * def2 * 20.0, 0.), 2.);
    float d = sdEllipse(glm::vec2(0.05, 0.11), uv);
    d = max(d, -headd + p.stroke);
    LayerS(d, glm::vec3(0.757, 0.772, 0.796), p.stroke);
    glm::vec2 uv2 = rotate(uv, -0.2);
    uv2 += glm::vec2(0.005, 0.026);
    float d2 = sdEllipse(glm::vec2(0.024, 0.045), uv2);
    d2 = max(d2, -headd + p.stroke);
    LayerS(d2, glm::vec3(0.49, 0.46, 0.51), p.stroke);
    {
    	glm::vec2 _uv = uv;
    	glm::vec2 plt = glm::vec2(-0.015, 0.085); glm::vec2 pmt = glm::vec2(-0.0, 0.085); glm::vec2 prt = glm::vec2(0.005, 0.035);
    	float du = udBezier(prt, pmt, plt, _uv);
    	LayerS(du, glm::vec3(0.),  p.stroke * 0.5);
    }
}

void make_ears(inout glm::vec4 fragColor, Params p, glm::vec2 uv, float headd)
{
    float l = float(uv.x > 0.);
    uv.x = -abs(uv.x);
    make_ear(fragColor, p, uv, l, headd);
}

void make_body_shadow(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv += glm::vec2(-0.045, 0.46);
    uv = rotate(uv, -0.05);
    uv += glm::vec2(-0.01, 0.1);
    float def = uv.y + uv.x * 0.2 - 0.06;
    uv.y += glm::pow(max(def, 0.), 2.) * 12.0;
	float d2 = sdUnevenCapsuleY(uv, 0.5, 0.26, 0.9 ) - 0.04;
    LayerS(d2 + 0.01, glm::vec3(0.31, 0.15, 0.16), p.stroke);
}

void make_body(inout glm::vec4 fragColor, Params p, glm::vec2 uv)
{
    uv += glm::vec2(-0.045, 0.46);
    uv = rotate(uv, -0.05);
    float d = sdTrapezoid(0.14, 0.36, 0.11, uv) - 0.04;
    
    uv += glm::vec2(-0.01, 0.1);
    
	float dw = sdTrapezoid(0.18, 0.30, 0.11, uv) - 0.04;
    float _dw = dw;
    glm::vec2 _uv = uv;
    
    float def = uv.y + uv.x * 0.2 - 0.06;
    uv.y += glm::pow(max(def, 0.), 2.) * 12.0;
    uv.y += 0.25 - glm::pow(abs(uv.x) * 1.2, 1.4);
	float d2 = sdUnevenCapsuleY(uv, 0.5, 0.26, 0.9 ) - 0.04;
    
    uv = _uv;
    def = uv.y + (uv.x - 0.1) * 0.2 - 0.06;
    uv.y += (min(max(uv.y + 0.7, 0.), 0.8) + 0.2) * (0.26 - min(glm::pow(abs(uv.x + uv.y * 0.2 - 0.03) * 1.2, 1.4), 0.26)) * 1.2;
    
	float d22 = sdUnevenCapsuleY(uv, 0.5, 0.26, 0.9 ) - 0.04;
    float dd = max(d, d2);
    dw = max(dw, d2);
    dw = max(dw, -d);
    float _dw_ = dw;
    dw = max(dw, -min(abs(_uv.x + _uv.y * 0.1 + 0.03) -0.02, 0.1));
    
    uv = rotate(_uv, 0.03);
    float def2 = glm::pow(max(-uv.y + uv.x * 0.075 + 0.06, 0.), 3.1);
    float k = 1.5;
    def2 = (1. / (1. + exp(k * def2* 470.0)) - 0.5) / k;
    uv.x += sign(uv.x) * def2;//min(glm::pow(max(def2, 0.), 2.) * 10.0, 0.23);
    uv.x += def2 *0.25;
	float d3 = sdUnevenCapsuleY(uv, 0.5, 0.26, 0.9 ) - 0.04;
    d3 = min(d3, d2);
    d3 = max(d3, -d);
    d3 = max(d3, uv.y - 0.05);
    
    float d33 = max(d3, abs(_uv.x - 0.04) - _uv.y * (0.3 + 4.*max(uv.x, 0.))  - 0.55);
    
    LayerSM(d2, glm::vec3(0.31, 0.15, 0.16), p.stroke * 2.0, -abs(_uv.x + 0.03) + 0.22);
    LayerF(d22, glm::vec3(0.48, 0.2, 0.21));
    LayerS(d3, glm::vec3(0.70, 0.77, 0.87) * 0.9, p.stroke);
    LayerS(d33, glm::vec3(0.70, 0.77, 0.87) * 0.7, p.stroke);
    LayerS(_dw_ + 0.005, glm::vec3(0.70, 0.77, 0.87), p.stroke);
    LayerS(dw, glm::vec3(0.70, 0.77, 0.87), p.stroke);
}

void make_background(inout glm::vec4 c, glm::vec2 uv)
{
    float R = 0.6;
	glm::vec2 grid;
    float uv_ys = uv.x / (1.5*R);
    grid.y = fract(uv_ys);
    float odd = mod(floor(uv_ys), 2.0);
    grid.x = fract(uv.y / (SQRT3 * R) - odd*.5) - 0.5;
    float d =  abs(grid.x);
    d = mix(1e3, d, grid.y > 1./3.);
    grid.x = abs(grid.x); 
    float dd1 = abs(dot(grid - glm::vec2(0, 1./3.), normalize(glm::vec2(1./ 3., 0.5))));
    grid.y = 1.0 - grid.y + 1. + 1./3.; 
    float dd2 = abs(dot(grid - glm::vec2(0, 4./3.), normalize(glm::vec2(1./ 3., 0.5))));
    d = min(dd1, d);
    d = min(dd2, d);
        
    float anti = fwidth2(d) * 1.0;
    float sig = 0.00005;
    float hex = 1. - exp(-d*d * 0.5 / sig) * 0.3;//glm::smoothstep(-anti, anti, d - stroke);
    glm::vec3 bcol = glm::vec3(0.3,0.4, 0.57) * (0.8 + 0.2*uv.y) *(1.1-0.1*length(uv));
    
    c = glm::vec4(bcol * (hex * 0.5 + 0.5), 1.0); 
}

void mainImage(out glm::vec4 fragColor, in glm::vec2 fragCoord )
{
    glm::vec2 uv = (fragCoord-0.5*iResolution.xy)/iResolution.y * 2.0;
    uv /= 1.4;
    uv += glm::vec2(0.3, 0.0);
    
    Params p;
	p.size = min(iResolution.x, iResolution.y);
    p.pixSize = 1.0 / p.size;
    p.yaw = 1.;
    p.lj = 0.3;
    p.wj = 0.21;
    p.wc = 0.29;
    p.th = 0.12;
    p.trh = 0.15;
    p.radius = 0.08;
    p.stroke = p.pixSize * 1.5 + disp(uv, 40.0) * 0.001;
    p.m = -p.lj + p.th + p.trh * 2. + p.radius / 2.0;
    
    make_background(fragColor, uv - glm::vec2(0., 0.5));
    
    uv = rotate(uv, 0.13);
    glm::vec2 uv_nw = uv;
    uv_nw.x -= 0.09 * p.yaw;
    uv.x -= 0.1 * p.yaw * (cos(uv.x / 0.35)) * (cos(max(uv.y - p.m, 0.0) / 0.35));
    
    make_hair_back(fragColor, p, uv_nw);
    make_body_shadow(fragColor, p, uv);
    make_neck(fragColor, p, uv);
    make_body(fragColor, p, uv);
    float headd = make_head(fragColor, p, uv);
    float d = disp(uv, 30.0) * 0.0015 * float(int(iTime * 12.0) % 2 == 0);
    make_mouth(fragColor, p, uv + d);
    make_nose(fragColor, p, uv_nw);
    make_eyebrows(fragColor, p, uv_nw + d);
    make_hair_shadow(fragColor, p, uv_nw);
    float olds = p.stroke;
    p.stroke = p.pixSize * 1.5 + d;
    make_eyes(fragColor, p, uv_nw + d);
    p.stroke = olds;
    make_hair(fragColor, p, uv_nw);
    make_ears(fragColor, p, uv_nw, headd);
    make_hair2(fragColor, p, uv_nw);
    make_band_and_horns(fragColor, p, uv_nw);
    float n = noise(uv_nw * 200.0);
    fragColor += n * 0.04;
}



/**
 * @brief Czech flag vertex shader
 *
 * @param outVertex out vertex
 * @param inVertex in vertex
 * @param uniforms uniform variables
 */
void vertexShader(OutVertex&outVertex,InVertex const&inVertex,ShaderInterface const&si){
  auto const& vp   = si.uniforms[0].m4;

  auto&vCoord = outVertex.attributes[0].v2;
  auto&vPos   = outVertex.attributes[1].v3;
  auto&voff   = outVertex.attributes[2].u1;

  auto gl_VertexID = inVertex.gl_VertexID;
  auto time = si.uniforms[1].v1;
  
  const glm::vec2 vertices[] = {
    glm::vec2(-1.f,-1.f),
    glm::vec2(+1.f,-1.f),
    glm::vec2(-1.f,+1.f),
    glm::vec2(-1.f,+1.f),
    glm::vec2(+1.f,-1.f),
    glm::vec2(+1.f,+1.f)
  };
  int vid = gl_VertexID % 6;
  int fid =  gl_VertexID / 6;
  voff = fid;
  glm::vec2 pos =  vertices[vid];
  vPos = glm::vec3(pos,0);
  outVertex.gl_Position = vp * glm::vec4(pos+glm::vec2(fid%10*2-10,fid/10*3-40),0.f,1.f);
  vCoord = pos*0.5f+0.5f;
}

glm::vec4 angryTexture(glm::vec2 uv,uint32_t voff,float iTime){
  float dist = distance(uv,glm::vec2(.5f,.5f));
  
  glm::vec2 mrk = glm::vec2(1,1+100*glm::pow(abs(sin(iTime*2+voff)),60.f));
  glm::vec2 mrk2 = glm::vec2(1,1+100*glm::pow(abs(sin(iTime*3+2+voff)),60.f));
  glm::vec2 ee1 = glm::vec2(.75f,.7f);
  glm::vec2 ee2 = glm::vec2(.25f,.7f);  
  float e1 = distance((uv-ee1)*mrk+ee1,ee1);
  float e2 = distance((uv-ee2)*mrk2+ee2,ee2);
  
  glm::vec2 mmp1 = glm::vec2(.5f,.23f);
  glm::vec2 mmp2 = glm::vec2(.5f,.23f+0.06*sin(iTime+voff*30)); 
  glm::vec2 mm1 =   glm::vec2(0.7,1);
  float m1 = distance((uv-mmp1)*mm1+mmp1,mmp1);
  float m2 = distance((uv-mmp2)*mm1+mmp2,mmp2); 
  if(e1<0.02+0.06*sin(voff))return glm::vec4(0,0,0,1);
  if(e2<0.02+0.06*sin(voff))return glm::vec4(0,0,0,1);
  if(e1<0.1)return glm::vec4(1);
  if(e2<0.1)return glm::vec4(1);
  if(m1<0.15+0.06*sin(voff) && m2> 0.16+0.06*sin(voff))return glm::vec4(0,0,0,1);
  glm::vec3 red    = glm::vec3(cos(voff*30),sin(voff*20),cos(voff*100))*.5f+.5f;
 
  return glm::vec4(red,float(dist < .5f));
}

/**
 * @brief Czech flag fragment shader
 *
 * @param outFragment output fragment
 * @param inFragment input fragment
 * @param uniforms uniform variables
 */
void fragmentShader(OutFragment&outFragment,InFragment const&inFragment,ShaderInterface const&si){
  auto const& vCoord = inFragment.attributes[0].v2;
  auto const& voff = inFragment.attributes[2].u1;
  auto iTime = si.uniforms[1].v1;
  outFragment.gl_FragColor= angryTexture(vCoord,voff,iTime);
}

Method::Method(MethodConstructionData const*){

  mem.programs[0].vertexShader   = vertexShader;
  mem.programs[0].fragmentShader = fragmentShader;
  mem.programs[0].vs2fs[0]       = AttributeType::VEC2;
  mem.programs[0].vs2fs[1]       = AttributeType::VEC3;
  mem.programs[0].vs2fs[2]       = AttributeType::UINT;

  pushClearCommand(commandBuffer,glm::vec4(.1,.1,.1,1));
  pushDrawCommand (commandBuffer,6*545,0);

}

void Method::onUpdate(float dt){
  time += dt;
}

void Method::onDraw(Frame&frame,SceneParam const&sceneParam){
  mem.framebuffer = frame;
  mem.uniforms[0].m4 = sceneParam.proj*sceneParam.view;
  mem.uniforms[1].v1 = time;
  gpu_execute(mem,commandBuffer);
}

EntryPoint main = [](){registerMethod<Method>("izg06 students when they see the izg project");};

}
