#pragma once

class bone_setup {
public:
	//bool build_bones( c_cs_player *target, matrix_3x4 *mat, int mask, vector_3d rotation, vector_3d origin, float time, std::array< float, 24 > &poses );
	//void build( c_cs_player *pl, matrix_3x4_aligned *bone_to_world, int mask, const vector_3d &origin, const vector_3d &angles, float time, const vector_3d &eye_angles );
	void build( Player* pl, BoneArray* bone_to_world, int mask, const vec3_t& origin, const ang_t& angles, float time, const ang_t& eye_angles );
};

//struct c_bone_setup {
//    c_bone_setup( const c_studio_hdr *studio_hdr, int bone_mask, float *pose_parameters );
//    void init_pose( vector_3d pos[], vector_4d q[] );
//    void accumulate_pose( vector_3d pos[], vector_4d q[], int sequence, float cycle, float weight, float time, void *IKContext );
//    void calc_autoplay_sequences( vector_3d pos[], vector_4d q[], float real_time, void *IKContext );
//    void calc_bone_adj( vector_3d pos[], vector_4d q[], const float controllers[] );
//
//    const c_studio_hdr *m_pStudioHdr;
//    int m_boneMask;
//    float *m_flPoseParameter;
//    void *m_pPoseDebugger;
//};
//
//struct i_bone_setup {
//    c_bone_setup *bone_setup;
//};
extern bone_setup g_bone_setup;
