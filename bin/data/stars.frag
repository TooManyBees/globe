#version 150

uniform int frameNo;
in vec4 varyingColor;
in float varyingFocus;
in float varyingMagnitude;
in float varyingConstellation;
out vec4 outputColor;

float invertedMagnitude = -1 * varyingMagnitude + 7;

// float falloff = 45;
// float intensity = pow(10, 0.2 * invertedMagnitude);
// float fadeIntensity = pow(10, 0.05 * invertedMagnitude);

float falloff = 100;
float intensity = pow(2.52, min(6, invertedMagnitude));
float fadeIntensity = pow(1.25, min(7, invertedMagnitude + 3));

vec4 star(vec4 color, float intensity, float focus, float dist) {
    vec4 point = color * intensity * (1 + focus) / (dist * dist * dist);
    vec4 fade = color * fadeIntensity * (1 + focus) / (dist); // / 2;
    return point + fade;
}

// Standard issue Stackoverflow Perlin noise with frame and star as inputs
float twinkle() {
    vec2 star = vec2(frameNo, gl_PrimitiveID);
    return 1 + 0.2 * fract(sin(dot(star.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float highlightConstellation() {
    return 1.0 + 10.0 * varyingConstellation;
}

float highlight() {
    return step(0.5, varyingConstellation) * 2.0 + step(varyingConstellation, 0.5) * varyingFocus;
}

void main() {
    float d = distance(vec2(0.5), gl_PointCoord.st) * falloff;
    float i = intensity;
    vec4 col = star(varyingColor, i, highlight(), d);
    // float a = smoothstep(0, 3, col.r + col.g + col.b);
    float a = smoothstep(vec3(0), vec3(1), col.rgb).g;
    // float a = length(smoothstep(vec3(0), vec3(1), col.rgb));
    // a = a / max(1, varyingMagnitude) * (1 + highlight());
    a = a / (1 + smoothstep(0, 7, varyingMagnitude) * 6) * (1 + highlight());
    outputColor = vec4(col.rgb, a) * twinkle();
}
