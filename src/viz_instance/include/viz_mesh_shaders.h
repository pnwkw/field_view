#ifndef VIZ_INSTANCE_VIZ_MESH_SHADERS_H
#define VIZ_INSTANCE_VIZ_MESH_SHADERS_H

namespace viz {
constexpr const char *mag_cheb = R"(
#define DIMENSIONS 3

#define SOL_Z_SEGS 32
#define SOL_P_SEGS 512
#define SOL_R_SEGS 4096

#define SOL_PARAMS 2048
#define SOL_ROWS 16384
#define SOL_COLUMNS 65536
#define SOL_COEFFS 131072

#define DIP_Z_SEGS 128
#define DIP_Y_SEGS 2048
#define DIP_X_SEGS 16384

#define DIP_PARAMS 2048
#define DIP_ROWS 16384
#define DIP_COLUMNS 65536
#define DIP_COEFFS 262144

#define MAX_CHEB_ORDER 32

layout(std430, binding = 1) restrict readonly buffer sol_segment_ssbo {
    float MinZ;
    float MaxZ;
    float MultiplicativeFactor;
    int ZSegments;
    float SegZSol[SOL_Z_SEGS];
    int BegSegPSol[SOL_Z_SEGS];
    int NSegPSol[SOL_Z_SEGS];
    float SegPSol[SOL_P_SEGS];
    int BegSegRSol[SOL_P_SEGS];
    int NSegRSol[SOL_P_SEGS];
    float SegRSol[SOL_R_SEGS];
    int SegIDSol[SOL_R_SEGS];
} sol_segment;

layout(std430, binding = 2) restrict readonly buffer dip_segment_ssbo {
    float MinZ;
    float MaxZ;
    float MultiplicativeFactor;
    int ZSegments;
    float SegZDip[DIP_Z_SEGS];
    int BegSegYDip[DIP_Z_SEGS];
    int NSegYDip[DIP_Z_SEGS];
    float SegYDip[DIP_Y_SEGS];
    int BegSegXDip[DIP_Y_SEGS];
    int NSegXDip[DIP_Y_SEGS];
    float SegXDip[DIP_X_SEGS];
    int SegIDDip[DIP_X_SEGS];
} dip_segment;

layout(std430, binding = 3) restrict readonly buffer sol_params_ssbo {
    float BOffsets[DIMENSIONS*SOL_PARAMS];
    float BScales[DIMENSIONS*SOL_PARAMS];
    float BMin[DIMENSIONS*SOL_PARAMS];
    float BMax[DIMENSIONS*SOL_PARAMS];
    int NRows[DIMENSIONS*SOL_PARAMS];
    int ColsAtRowOffset[DIMENSIONS*SOL_PARAMS];
    int CofsAtRowOffset[DIMENSIONS*SOL_PARAMS];
    int NColsAtRow[SOL_ROWS];
    int CofsAtColOffset[SOL_ROWS];
    int NCofsAtCol[SOL_COLUMNS];
    int AtColCoefOffset[SOL_COLUMNS];
    float Coeffs[SOL_COEFFS];
} sol_params;

layout(std430, binding = 4) restrict readonly buffer dip_params_ssbo {
    float BOffsets[DIMENSIONS*DIP_PARAMS];
    float BScales[DIMENSIONS*DIP_PARAMS];
    float BMin[DIMENSIONS*DIP_PARAMS];
    float BMax[DIMENSIONS*DIP_PARAMS];
    int NRows[DIMENSIONS*DIP_PARAMS];
    int ColsAtRowOffset[DIMENSIONS*DIP_PARAMS];
    int CofsAtRowOffset[DIMENSIONS*DIP_PARAMS];
    int NColsAtRow[DIP_ROWS];
    int CofsAtColOffset[DIP_ROWS];
    int NCofsAtCol[DIP_COLUMNS];
    int AtColCoefOffset[DIP_COLUMNS];
    float Coeffs[DIP_COEFFS];
} dip_params;

float tmpCfs1[MAX_CHEB_ORDER];
float tmpCfs0[MAX_CHEB_ORDER];

vec3 CarttoCyl(vec3 pos) {
    return vec3(length(pos.xy), atan(pos.y, pos.x), pos.z);
}

int findSolSegment(vec3 pos) {
    int rid,pid,zid;
    for(zid=0; zid < sol_segment.ZSegments; zid++) if(pos.z<sol_segment.SegZSol[zid]) break;
    if(--zid < 0) zid = 0;
    const int psegBeg = sol_segment.BegSegPSol[zid];
    for(pid=0; pid<sol_segment.NSegPSol[zid]; pid++) if(pos.y<sol_segment.SegPSol[psegBeg+pid]) break;
    if(--pid < 0) pid = 0;
    pid += psegBeg;
    const int rsegBeg = sol_segment.BegSegRSol[pid];
    for(rid=0; rid<sol_segment.NSegRSol[pid]; rid++) if(pos.x<sol_segment.SegRSol[rsegBeg+rid]) break;
    if(--rid < 0) rid = 0;
    rid += rsegBeg;
    return sol_segment.SegIDSol[rid];
}

int findDipSegment(vec3 pos) {
    int xid,yid,zid;
    for(zid=0; zid < dip_segment.ZSegments; zid++) if(pos.z<dip_segment.SegZDip[zid]) break;
    if(--zid < 0) zid = 0;
    const int ysegBeg = dip_segment.BegSegYDip[zid];
    for(yid=0; yid<dip_segment.NSegYDip[zid]; yid++) if(pos.y<dip_segment.SegYDip[ysegBeg+yid]) break;
    if(--yid < 0) yid = 0;
    yid += ysegBeg;
    const int xsegBeg = dip_segment.BegSegXDip[yid];
    for(xid=0; xid<dip_segment.NSegXDip[yid]; xid++) if(pos.x<dip_segment.SegXDip[xsegBeg+xid]) break;
    if(--xid < 0) xid = 0;
    xid += xsegBeg;
    return dip_segment.SegIDDip[xid];
}

vec3 mapToInternalSol(int segID, vec3 rphiz) {
    const int index = DIMENSIONS*segID;
    vec3 offsets = vec3(sol_params.BOffsets[index+0], sol_params.BOffsets[index+1], sol_params.BOffsets[index+2]);
    vec3 scales = vec3(sol_params.BScales[index+0], sol_params.BScales[index+1], sol_params.BScales[index+2]);
    return (rphiz-offsets)*scales;
}

vec3 mapToInternalDip(int segID, vec3 pos) {
    const int index = DIMENSIONS*segID;
    const vec3 offsets = vec3(dip_params.BOffsets[index+0], dip_params.BOffsets[index+1], dip_params.BOffsets[index+2]);
    const vec3 scales = vec3(dip_params.BScales[index+0], dip_params.BScales[index+1], dip_params.BScales[index+2]);
    return (pos-offsets)*scales;
}

float cheb1DArray(float x, float arr[MAX_CHEB_ORDER], int ncf) {
    if(ncf <= 0) return 0.0f;
    const float x2 = 2*x;
    vec3 b = vec3(arr[--ncf], 0, 0);
    --ncf;
    const vec3 t1 = vec3(1, x2, -1);
    for (int i=ncf;i>=0;i--) {
        b.zy = b.yx;
        b.x = arr[i];
        b.x = dot(t1, b);
    }
    const vec3 t = vec3(1, -x, 0);
    return dot(t, b);
}

float cheb1DParamsSol(float x, int coeff_offset, int ncf) {
    if(ncf <= 0) return 0.0f;
    const float x2 = 2*x;
    vec3 b = vec3(sol_params.Coeffs[coeff_offset + (--ncf)], 0, 0);
    --ncf;
    const vec3 t1 = vec3(1, x2, -1);
    for (int i=ncf;i>=0;i--) {
        b.zy = b.yx;
        b.x = sol_params.Coeffs[coeff_offset + i];
        b.x = dot(t1, b);
    }
    const vec3 t = vec3(1, -x, 0);
    return dot(t, b);
}

float cheb1DParamsDip(float x, int coeff_offset, int ncf) {
    if(ncf <= 0) return 0.0f;
    const float x2 = 2*x;
    vec3 b = vec3(dip_params.Coeffs[coeff_offset + (--ncf)], 0, 0);
    --ncf;
    const vec3 t1 = vec3(1, x2, -1);
    for (int i=ncf;i>=0;i--) {
        b.zy = b.yx;
        b.x = dip_params.Coeffs[coeff_offset + i];
        b.x = dot(t1, b);
    }
    const vec3 t = vec3(1, -x, 0);
    return dot(t, b);
}

bool IsBetween(vec3 sMin, vec3 val, vec3 sMax) {
    return all(lessThanEqual(sMin, val)) && all(lessThanEqual(val, sMax));
}

bool IsInsideSol(int segID, vec3 rphiz) {
    const int index = DIMENSIONS*segID;
    const vec3 seg_min = vec3(sol_params.BMin[index+0], sol_params.BMin[index+1], sol_params.BMin[index+2]);
    const vec3 seg_max = vec3(sol_params.BMax[index+0], sol_params.BMax[index+1], sol_params.BMax[index+2]);
    return IsBetween(seg_min, rphiz, seg_max);
}

bool IsInsideDip(int segID, vec3 pos) {
    const int index = DIMENSIONS*segID;
    const vec3 seg_min = vec3(dip_params.BMin[index+0], dip_params.BMin[index+1], dip_params.BMin[index+2]);
    const vec3 seg_max = vec3(dip_params.BMax[index+0], dip_params.BMax[index+1], dip_params.BMax[index+2]);
    return IsBetween(seg_min, pos, seg_max);
}

float Eval3DSol(int segID, int dim, vec3 internal) {
    const int index = DIMENSIONS*segID;
    const int n_rows = sol_params.NRows[index+dim];
    const int cols_at_row_offset = sol_params.ColsAtRowOffset[index+dim];
    const int coeffs_at_row_offset = sol_params.CofsAtRowOffset[index+dim];
    for(int row = 0; row < n_rows; row++) {
        const int n_cols = sol_params.NColsAtRow[cols_at_row_offset+row];
        const int coeff_at_col_offset = sol_params.CofsAtColOffset[cols_at_row_offset+row];
        for(int col = 0; col < n_cols; col++) {
            const int n_coeffs = sol_params.NCofsAtCol[coeff_at_col_offset+col];
            const int per_col_coeff_offset = sol_params.AtColCoefOffset[coeff_at_col_offset+col];
            const int coeffs_offset = coeffs_at_row_offset + per_col_coeff_offset;
            tmpCfs1[col] = cheb1DParamsSol(internal.z, coeffs_offset,n_coeffs);
        }
        tmpCfs0[row] = cheb1DArray(internal.y, tmpCfs1, n_cols);
    }
    return cheb1DArray(internal.x, tmpCfs0, n_rows);
}

vec3 EvalSol(int segID, vec3 rphiz) {
    const vec3 internal = mapToInternalSol(segID, rphiz);
    return vec3(Eval3DSol(segID, 0, internal), Eval3DSol(segID, 1, internal), Eval3DSol(segID, 2, internal));
}

float Eval3DDip(int segID, int dim, vec3 internal) {
    const int index = DIMENSIONS*segID;
    const int n_rows = dip_params.NRows[index+dim];
    const int cols_at_row_offset = dip_params.ColsAtRowOffset[index+dim];
    const int coeffs_at_row_offset = dip_params.CofsAtRowOffset[index+dim];
    for(int row = 0; row < n_rows; row++) {
        const int n_cols = dip_params.NColsAtRow[cols_at_row_offset+row];
        const int coeff_at_col_offset = dip_params.CofsAtColOffset[cols_at_row_offset+row];
        for(int col = 0; col < n_cols; col++) {
            const int n_coeffs = dip_params.NCofsAtCol[coeff_at_col_offset+col];
            const int per_col_coeff_offset = dip_params.AtColCoefOffset[coeff_at_col_offset+col];
            const int coeffs_offset = coeffs_at_row_offset + per_col_coeff_offset;
            tmpCfs1[col] = cheb1DParamsDip(internal.z, coeffs_offset, n_coeffs);
        }
        tmpCfs0[row] = cheb1DArray(internal.y, tmpCfs1, n_cols);
    }
    return cheb1DArray(internal.x, tmpCfs0, n_rows);
}

vec3 EvalDip(int segID, vec3 pos) {
    const vec3 internal = mapToInternalDip(segID, pos);
    return vec3(Eval3DDip(segID, 0, internal), Eval3DDip(segID, 1, internal), Eval3DDip(segID, 2, internal));
}

vec3 CyltoCartCylB(vec3 rphiz, vec3 brphiz) {
    float btr = length(brphiz.xy);
    float psiPLUSphi = atan(brphiz.y, brphiz.x) + rphiz.y;

    return vec3(btr*cos(psiPLUSphi), btr*sin(psiPLUSphi), brphiz.z);
}

vec3 MachineField(vec3 pos) {
    return vec3(0);
}

vec3 SolDipField(vec3 pos) {
    if(pos.z > sol_segment.MinZ) {
        const vec3 rphiz = CarttoCyl(pos);
        const int segID = findSolSegment(rphiz);
        if(segID >=0 && IsInsideSol(segID, rphiz)) {
            const vec3 brphiz = EvalSol(segID, rphiz);
            return CyltoCartCylB(rphiz, brphiz) * sol_segment.MultiplicativeFactor;
        }
    }
    const int segID = findDipSegment(pos);
    if(segID >= 0 && IsInsideDip(segID, pos)) {
        return EvalDip(segID, pos) * dip_segment.MultiplicativeFactor;
    }
    return vec3(0);
}

const float MinZ = dip_segment.MinZ;
const float MaxZ = sol_segment.MaxZ;

vec3 ALICEField(vec3 pos) {
    if(pos.z > MinZ && pos.z < MaxZ) {
        return SolDipField(pos);
    }
    return vec3(0);
}

)";

constexpr const char *common_fragment =
R"(#version 450

layout (location = 0) in PerVertexData {
    vec4 color;
} fragIn;

layout (location = 0) out vec4 FragColor;

void main() {
    FragColor = fragIn.color;
}
)";

constexpr const char *lines_ribbons_task =
R"(#version 450
#extension GL_NV_mesh_shader : require

layout(local_size_x=1) in;

taskNV out Task {
    uint vertexID;
} OUT;

void main() {
    gl_TaskCountNV = 1;
    OUT.vertexID = gl_WorkGroupID.x;
}
)";

constexpr const char *common_header =
R"(#version 450
#extension GL_NV_mesh_shader : require

)";

constexpr const char *field_func_template =
R"(vec3 Field(vec3 pos) {
    return ALICEField(pos);
}
)";

constexpr const char *curl_func = R"(
vec3 Curl(vec3 pos) {
    const float e = 1.0f;

    const vec2 dx_yz = (Field(pos + vec3(e,0,0)).yz - Field(pos - vec3(-e,0,0)).yz) / 2*e;
    const vec2 dy_xz = (Field(pos + vec3(0,e,0)).xz - Field(pos + vec3(0,-e,0)).xz) / 2*e;
    const vec2 dz_xy = (Field(pos + vec3(0,0,e)).xy - Field(pos - vec3(0,0,-e)).xy) / 2*e;

    return vec3(dy_xz[1] - dz_xy[1], dz_xy[0] - dx_yz[1], dx_yz[0] - dy_xz[0]);
}
)";

constexpr const char *div_func = R"(
float Divergence(vec3 pos) {
    const float e = 1.0f;

    const float x = (Field(pos + vec3(e,0,0)) - Field(pos - vec3(-e,0,0))).x / 2*e;
    const float y = (Field(pos + vec3(0,e,0)) - Field(pos - vec3(0,-e,0))).y / 2*e;
    const float z = (Field(pos + vec3(0,0,e)) - Field(pos - vec3(0,0,-e))).z / 2*e;

    return x + y + z;
}
)";

constexpr const char *lines_mesh_main = R"(
#define MAX_VERTICES 255

layout(local_size_x=2) in;
layout(lines, max_vertices=MAX_VERTICES, max_primitives=MAX_VERTICES-1) out;

out PerVertexData {
    vec4 color;
} v_out[];

layout(std140, binding = 0) uniform state_state {
    mat4 MVP;
} state;

layout(std430, binding = 0) restrict readonly buffer settings_settings {
    uint verticesCount;
    float step;
} settings;

layout(std430, binding = 5) restrict readonly buffer vertices_vertices {
    vec4 pos[];
} vertices;

taskNV in Task {
    uint vertexID;
} IN;

void main() {
    const uint thread_id = gl_LocalInvocationID.x;

    // map thread id -(0,1) to direction - (-1,1) the current thread will "walk" along the field line
    const int direction = int((thread_id * 2) - 1);

    const uint centerIndex = (settings.verticesCount / 2);

    vec4 position = vertices.pos[IN.vertexID];

    gl_MeshVerticesNV[centerIndex].gl_Position = state.MVP * position;
    v_out[centerIndex].color = vec4(1.0, 0.0, 0.0, 1.0);

    for (uint i = 0; i < centerIndex; ++i) {
        const int offsetFromCenter = int(i+1) * direction;

        const vec3 b_vec = Field(position.xyz);
        position.xyz += direction * b_vec * settings.step;

        gl_MeshVerticesNV[centerIndex + offsetFromCenter].gl_Position = state.MVP * position;

        vec4 color;

        if (thread_id == 0) {
            color = vec4(1.0, 0.0, 0.0, 1.0);
        } else {
            color = vec4(0.0, 0.0, 1.0, 1.0);
        }

        v_out[centerIndex + offsetFromCenter].color = color;

        gl_PrimitiveIndicesNV[i*4 + thread_id] = i*2 + thread_id;
        gl_PrimitiveIndicesNV[i*4 + 2 + thread_id] = i*2 + thread_id + 1;
    }

    gl_PrimitiveCountNV = settings.verticesCount-1;
}
)";

constexpr const char *ribbons_mesh_main = R"(
#define MAX_VERTICES 256

layout(local_size_x=4) in;
layout(triangles, max_vertices=MAX_VERTICES, max_primitives=MAX_VERTICES-2) out;

out PerVertexData {
    vec4 color;
} v_out[];

layout(std140, binding = 0) uniform state_state {
    mat4 MVP;
} state;

layout(std430, binding = 0) restrict readonly buffer settings_settings {
    uint segmentsCount;
    float initialOffset;
    float stepSize;
} settings;

layout(std430, binding = 5) restrict readonly buffer vertices_vertices {
    vec4 pos[];
} vertices;

taskNV in Task {
    uint vertexID;
} IN;

void main() {
    const uint maxvertices = settings.segmentsCount * 2 + 2;
    const uint thread_id = gl_LocalInvocationID.x;
    const uint firstOrSecondPair = thread_id >> 1;
    const uint isOdd = thread_id & 1u;

    // map evenOrOdd -(0,1) to direction - (-1,1) the current thread will "walk" along the field line
    const int direction = int((isOdd * 2) - 1);

    // 0  2  (4)  6  8
    //
    // 1  3  (5)  7  9
    // Figure out the first vertex id
    const uint centerIndex = (maxvertices / 2) - firstOrSecondPair; // 5 or 4 in the sample case

    vec4 position = vertices.pos[IN.vertexID];

    vec4 color;

    if (firstOrSecondPair == 0) {
        position.y -= settings.initialOffset / 2;
        color = vec4(1.0, 0.0, 0.0, 1.0);
    } else {
        position.y += settings.initialOffset / 2;
        color = vec4(0.0, 0.0, 1.0, 1.0);
    }

    gl_MeshVerticesNV[centerIndex].gl_Position = state.MVP * position;
    v_out[centerIndex].color = color;

    for (uint i = 0; i < (maxvertices / 4); ++i) {
        const int offsetFromCenter = int(i+1) * direction * 2;

        const vec3 b_vec = Field(position.xyz);
        position.xyz += direction * b_vec * settings.stepSize;

        gl_MeshVerticesNV[centerIndex + offsetFromCenter].gl_Position = state.MVP * position;
        v_out[centerIndex + offsetFromCenter].color = color;
    }

    const uint isFirst = int(thread_id == 0);
    const uint isSecond = int(thread_id == 1);
    const uint isThird = int(thread_id == 2);

    if (thread_id == 0 || thread_id == 1 || thread_id == 2) {
        for (uint i = 0; i < (maxvertices-2); ++i) {
            const uint index = 3 * i + isSecond + 2*isThird;
            const uint value = ((i+2)/2)*2 * isFirst + i * isSecond + (((i+1)/2)*2+1) * isThird;

            gl_PrimitiveIndicesNV[index] = value;
        }
    }

    gl_PrimitiveCountNV = maxvertices-2;
}
)";

constexpr const char *tubes_constants = R"(
#define MAX_SEGMENTS 255
#define MAX_CIRCLE_VERTICES 16
#define PI 3.1415926538
)";

constexpr const char *tubes_task_main = R"(
layout(local_size_x=2) in;

layout(std430, binding = 5) restrict readonly buffer vertices_vertices {
    vec4 pos[];
} vertices;

layout(std430, binding = 0) restrict readonly buffer settings_settings {
    uint segmentsCount;
    uint circleVertices;
    float radius;
    float stepSize;
} settings;

taskNV out Task {
    vec4 vertices[MAX_SEGMENTS+1];
} OUT;

void main() {
    const uint thread_id = gl_LocalInvocationID.x;

    // map thread id -(0,1) to direction - (-1,1) the current thread will "walk" along the line
    const int direction = int((thread_id * 2) - 1);

    const uint centerIndex = ((settings.segmentsCount + 1) / 2);

    vec4 position = vertices.pos[gl_WorkGroupID.x];
    vec3 b_vec = Field(position.xyz);

    OUT.vertices[centerIndex] = position;

    for (uint i = 0; i < centerIndex; ++i) {
        const int offsetFromCenter = int(i+1) * direction;

        position.xyz += direction * b_vec * settings.stepSize;

        b_vec = Field(position.xyz);

        OUT.vertices[centerIndex + offsetFromCenter] = position;
    }

    gl_TaskCountNV = settings.segmentsCount;
}
)";

constexpr const char *tubes_mesh_main = R"(
layout(local_size_x=3) in;
layout(triangles, max_vertices=2*MAX_CIRCLE_VERTICES, max_primitives=2*MAX_CIRCLE_VERTICES) out;

out PerVertexData {
    vec4 color;
    vec4 normal;
} v_out[];

layout(std140, binding = 0) uniform state_state {
    mat4 MVP;
} state;

taskNV in Task {
    vec4 vertices[MAX_SEGMENTS+1];
} IN;

layout(std430, binding = 0) restrict readonly buffer settings_settings {
    uint segmentsCount;
    uint circleVertices;
    float radius;
    float stepSize;
} settings;

void main() {
    const float angle_chunk = 2 * PI / float(settings.circleVertices);

    const uint thread_id = gl_LocalInvocationID.x;

    if (thread_id == 0 || thread_id == 1) {
        const uint vertex_index = gl_WorkGroupID.x;
        const vec4 position = IN.vertices[vertex_index + thread_id];
        const vec3 b_vec = Field(position.xyz);

        const vec4 tangent = vec4(normalize(b_vec), 0.0f);
        const vec4 normal = vec4(normalize(Curl(position.xyz)), 0.0f);
        const vec4 binormal = vec4(normalize(cross(normal.xyz, tangent.xyz)), 0.0f);
        const float radius = (length(b_vec) + 1.0f)/6.0f * settings.radius;

        for (uint i = 0; i < settings.circleVertices; ++i) {
            const float angle = angle_chunk * i;

            const vec4 pointPosition = position + binormal*radius*sin(angle) + normal*radius*cos(angle);

            if (thread_id == 0) {
                v_out[2 * i + thread_id].color = vec4(1.0, 0.0, 0.0, 1.0);
            } else {
                v_out[2 * i + thread_id].color = vec4(0.0, 0.0, 1.0, 1.0);
            }

            gl_MeshVerticesNV[2 * i + thread_id].gl_Position = state.MVP * pointPosition;
        }
    }

    const uint isFirst = uint(thread_id == 0);
    const uint isSecond = uint(thread_id == 1);
    const uint isThird = uint(thread_id == 2);

    for (uint i = 0; i < 2*settings.circleVertices-1; ++i) {
        const uint index = 3 * i + isSecond + 2*isThird;
        const uint value = i * isFirst + ((i+2)/2)*2 * isSecond + (((i+1)/2)*2+1) * isThird;

        gl_PrimitiveIndicesNV[index] = value;
    }

    const uint value1 = (2 * settings.circleVertices - 2) * isFirst + (2 * settings.circleVertices - 1) * isThird; // + 0 * isSecond
    const uint value2 = (2 * settings.circleVertices - 1) * isFirst + 1 * isThird; // + 0 * isSecond

    gl_PrimitiveIndicesNV[3 * (2 * settings.circleVertices - 2) + isSecond + 2 * isThird] = value1;
    gl_PrimitiveIndicesNV[3 * (2 * settings.circleVertices - 1) + isSecond + 2 * isThird] = value2;

    gl_PrimitiveCountNV = 2 * settings.circleVertices;
}
)";

constexpr const char *tubes_fragment = R"(
#version 450

layout (location = 0) in PerVertexData {
    vec4 color;
    vec4 normal;
} fragIn;

perprimitiveNV in something {
    vec4 color;
} block;

layout (location = 0) out vec4 FragColor;

void main() {
    FragColor = fragIn.color;
}
)";

constexpr const char *curiosity_func_template =
R"(float curiosityLevel(vec3 pos) {
    return length(Field(pos));
}
)";

constexpr const char *surface_constants = R"(
#define MAX_TASK_THREADS 32
#define RESOLUTION 16
#define LEVELS 3
)";

constexpr const char *surface_task_main = R"(
layout(local_size_x=MAX_TASK_THREADS) in;

layout(std430, binding = 5) restrict readonly buffer vertices_vertices {
    vec4 pos[];
} vertices;

layout(std430, binding = 6) restrict buffer samples_samples {
    float samples[];
} samples;

layout(std430, binding = 7) restrict buffer keep_keep {
    uint keep_levels[];
} keep_levels;

layout(std430, binding = 8) restrict writeonly buffer level_level {
    uint mesh_level[];
} mesh_level;

layout(std430, binding = 9) restrict writeonly buffer index_index {
    uint mesh_index[];
} mesh_index;

layout(std430, binding = 0) restrict readonly buffer settings_settings {
    float threshold;
} settings;

taskNV out Task {
    uint vertexID;
} OUT;

uint level_length(uint level) {
    return 1 << ((LEVELS-level+1) << 1);
}

uint levels_index(uint level) {
    uint index = 0;

    for (uint i = 0; i < level; ++i) {
        index += level_length(i);
    }

    return index;
}

void process_level(float threshold, uint level, uint i) {
    //coord = i * (4 ^ (level + 1))
    const uint coord = i * (1 << ((level + 1) << 1));
    const uint current_level_index = levels_index(level);
    const uint next_level_index = levels_index(level + 1);

    bool stop_concatenate = false;

    if (level != 0) {
        // don't try to concatenate if one or more smaller squares were not concatenated
        for (uint j = 0; j < 4; ++j) {
            const uint idx = current_level_index + i * 4 + j;
            if (keep_levels.keep_levels[idx] == 0) {
                stop_concatenate = true;
                break;
            }
        }
    }

    if (!stop_concatenate) {
        // calculate average interestness
        float average = 0.0f;

        //multiplier = 4 ^ level
        const uint multiplier = 1 << (level << 1);

        for (uint j = 0; j < 4; ++j) {
            average += samples.samples[coord + j * multiplier];
        }

        average /= 4;

        // not attractive enough
        if (average < threshold) {
            samples.samples[coord] = average;

            uint level_idx = i * multiplier;

            if (level != 0) {
                const uint idx = current_level_index + level_idx;
                keep_levels.keep_levels[idx] = 0;
            }

            if (level == 0) {
                level_idx = coord;
            }

            for (uint j = 1; j < 4; ++j) {
                samples.samples[coord + j * multiplier] = 0;
                const uint idx = current_level_index + level_idx + j;
                keep_levels.keep_levels[idx] = 0;
            }

            const uint idx = next_level_index + i;
            keep_levels.keep_levels[idx] = 1;
        } else if (level == 0) {
            for (uint j = 0; j < 4; ++j) {
                const uint idx = current_level_index + coord + j;
                keep_levels.keep_levels[idx] = 1;
            }
        }
    }
}

vec4 calcPosition(float x_frac, float y_frac) {
    const vec4 vl = mix(vertices.pos[0], vertices.pos[1], y_frac);
    const vec4 vr = mix(vertices.pos[2], vertices.pos[3], y_frac);

    return mix(vl, vr, x_frac);
}

uint compactBy1(uint x) {
    x &= 0x55555555;                 // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    x = (x ^ (x >> 1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
    x = (x ^ (x >> 2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
    x = (x ^ (x >> 4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
    x = (x ^ (x >> 8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210

    return x;
}

float curiosityLevelFor(uint sample_idx) {
    const uint morton_x = compactBy1(sample_idx);
    const uint morton_y = compactBy1(sample_idx >> 1);

    const uint max_fragments = 2 * RESOLUTION;
    const float frac = 1.0f / max_fragments;

    const float x_frac = frac * (morton_x * 2 + 1);
    const float y_frac = frac * (morton_y * 2 + 1);

    const vec3 center = calcPosition(x_frac, y_frac).xyz;

    return curiosityLevel(center);
}

void main() {
    const uint thread_id = gl_LocalInvocationID.x;

    for (uint i = 0; i < (RESOLUTION*RESOLUTION)/MAX_TASK_THREADS; ++i) {
        const uint sample_idx = i * MAX_TASK_THREADS + thread_id;
        samples.samples[sample_idx] = curiosityLevelFor(sample_idx);
    }

    const float threshold = settings.threshold;

    for (uint i_l = 0; i_l < LEVELS; ++i_l) {
        if ((level_length(i_l)/4) >= MAX_TASK_THREADS) {
            for (uint i = 0; i < level_length(i_l)/ (4 * MAX_TASK_THREADS); ++i) {
                process_level(threshold, i_l, i * MAX_TASK_THREADS + thread_id);
            }
        } else if (thread_id < (level_length(i_l)/4)) {
            process_level(threshold, i_l, thread_id);
        }
    }

    if (thread_id == 0) {
        uint mesh_count = 0;
        for(uint i_l = 0; i_l < LEVELS+1; ++i_l) {
            for(uint x = 0; x < level_length(i_l); ++x) {
                const uint idx = levels_index(i_l) + x;

                if (keep_levels.keep_levels[idx] != 0) {
                    mesh_level.mesh_level[mesh_count] = i_l;
                    mesh_index.mesh_index[mesh_count] = x;
                    mesh_count++;
                }
            }
        }

        gl_TaskCountNV = mesh_count;
    }
}
)";

constexpr const char *surface_mesh_main = R"(
layout(local_size_x=4) in;
layout(triangles, max_vertices=4, max_primitives=2) out;

out PerVertexData {
    vec4 color;
} v_out[];

layout(std140, binding = 0) uniform state_state {
    mat4 MVP;
} state;

layout(std430, binding = 5) restrict readonly buffer vertices_vertices {
    vec4 pos[];
} vertices;

layout(std430, binding = 8) restrict readonly buffer level_level {
    uint mesh_level[];
} mesh_level;

layout(std430, binding = 9) restrict readonly buffer index_index {
    uint mesh_index[];
} mesh_index;

taskNV in Task {
    uint vertexID;
} IN;

uint compactBy1(uint x) {
    x &= 0x55555555;                 // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    x = (x ^ (x >> 1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
    x = (x ^ (x >> 2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
    x = (x ^ (x >> 4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
    x = (x ^ (x >> 8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210

    return x;
}

uint level_resolution(uint level) {
    return RESOLUTION / (1 << level);
}

vec4 calcPosition(float x_frac, float y_frac) {
    const vec4 vl = mix(vertices.pos[0], vertices.pos[1], y_frac);
    const vec4 vr = mix(vertices.pos[2], vertices.pos[3], y_frac);

    return mix(vl, vr, x_frac);
}

vec4 calcColor(float x_frac, float y_frac) {
    const vec4 cl = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), y_frac);
    const vec4 cr = mix(vec4(0.0, 0.0, 1.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0), y_frac);

    vec3 color = mix(cl, cr, x_frac).rgb;
    vec3 field = Field(calcPosition(x_frac, y_frac).xyz)/5.0f;
    float len = length(field);

    return vec4(len*color, 1);
}

const float offsets_x[] = {-1.0f, -1.0f, 1.0f,  1.0f};
const float offsets_y[] = {-1.0f,  1.0f, 1.0f, -1.0f};

void main() {
    const uint thread_id = gl_LocalInvocationID.x;
    const uint group_id = gl_WorkGroupID.x;
    const uint level = mesh_level.mesh_level[group_id];
    const uint index = mesh_index.mesh_index[group_id];

    const uint res = level_resolution(level);
    const uint max_fragments = 2 * res;
    const float frac = 1.0f / max_fragments;

    // Use index to generate morton code
    const uint morton_x = compactBy1(index);
    const uint morton_y = compactBy1(index >> 1);
    const float x_frac = frac * (morton_x * 2 + 1);
    const float y_frac = frac * (morton_y * 2 + 1);
    const vec4 color = calcColor(x_frac, y_frac);

    const float offsetx = frac * offsets_x[thread_id];
    const float offsety = frac * offsets_y[thread_id];

    gl_MeshVerticesNV[thread_id].gl_Position = state.MVP * calcPosition(x_frac + offsetx, y_frac + offsety);
    v_out[thread_id].color = color;

    if (thread_id < 3) {
        const uint isSecondThird = uint(thread_id > 0);

        gl_PrimitiveIndicesNV[thread_id] = thread_id;
        gl_PrimitiveIndicesNV[3 + thread_id] = thread_id + isSecondThird;
    }

    gl_PrimitiveCountNV = 2;
}
)";
}// namespace viz

#endif //VIZ_INSTANCE_VIZ_MESH_SHADERS_H
