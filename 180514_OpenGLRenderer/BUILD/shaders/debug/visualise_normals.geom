#version 440 core

// Input primitives
layout(triangles) in;

// Output primitives
layout(line_strip, max_vertices = 18) out;		// 18 vertices representing each point for the collective normal (6), bitangent (6) and tangent lines (6)

// Data from vertex shader (arrays because taking data from 3 verts (triangle layout))
in vec3 worldNormal[];
in vec3 worldTangent[];
flat in float bitangentHandedness[]; 

uniform mat4 modelTransform;		// Global space
uniform mat4 viewTransform;			// View space
uniform mat4 projectionTransform;	// Clip space

uniform float drawScale;		// How big to draw the debug normals

uniform vec4 normalColor;
uniform vec4 tangentColor;
uniform vec4 bitangentColor;

// To the frag shader
out vec4 outputColor;

void main() {
	mat4 projectionView = projectionTransform * viewTransform;

	// Re-orthogonalize tangents, and calculate bitangent
	vec3 worldBitangent[3];
	for (int i = 0; i < 3; i++) {
			vec3 N = normalize(worldNormal[i]);
			vec3 T = normalize(worldTangent[i]);
			
			// Ensure tangent is orthogonal to the normal (90 degrees) by re-orthogonalizing
			T = normalize(T - dot(T, N) * N);

			worldBitangent[i] = cross(N, T) * bitangentHandedness[i];		// Get unknown up axis by getting the cross between the right (tangent)
	}

	// Get vertex normals in world space
	vec3 vertNormalsW[3];
	vertNormalsW[0] = worldNormal[0];
	vertNormalsW[1] = worldNormal[1];
	vertNormalsW[2] = worldNormal[2];

	// Convert vertex input positions into clipping space (slightly shifted along normal to avoid z fighting with the mesh)
	vec4 vertPositionsCl[3];
	vertPositionsCl[0] = projectionView * vec4(gl_in[0].gl_Position.xyz + vertNormalsW[0] * 0.01, 1.0);
	vertPositionsCl[1] = projectionView * vec4(gl_in[1].gl_Position.xyz + vertNormalsW[1] * 0.01, 1.0);
	vertPositionsCl[2] = projectionView * vec4(gl_in[2].gl_Position.xyz + vertNormalsW[2] * 0.01, 1.0);

	//// Create normal, tangent and bitangent representation for each vertex in the triangle (DRAWN IN CLIPPING SPACE)
	for (int i = 0; i < gl_in.length(); i++) {
		// Get vertex position in world space
		vec3 vertPosW = gl_in[i].gl_Position.xyz;

		// Draw normal
		outputColor = normalColor;

		gl_Position = vertPositionsCl[i];		// Vert position to
		EmitVertex();

		gl_Position = projectionView * vec4(vertPosW + worldNormal[i].xyz * drawScale, 1.0);		// Head of normal
		EmitVertex();

		EndPrimitive();

		// Draw tangent
		outputColor = tangentColor;

		gl_Position = vertPositionsCl[i];
		EmitVertex();

		gl_Position = projectionView * vec4(vertPosW + worldTangent[i].xyz * drawScale, 1.0);
		EmitVertex();

		EndPrimitive();

		// Draw bitangent
		outputColor = bitangentColor;

		gl_Position = vertPositionsCl[i];
		EmitVertex();

		gl_Position = projectionView * vec4(vertPosW + worldBitangent[i].xyz * drawScale, 1.0);
		EmitVertex();

		EndPrimitive();

	}
}
