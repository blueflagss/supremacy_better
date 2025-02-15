#pragma once

class CIKContext {
    PAD( 0x1070 );

public:
    CIKContext( );

    void Init( const CStudioHdr *hdr, const ang_t &local_angles, const vec3_t &local_origin, float current_time, int frame_count, int bone_mask );
    void UpdateTargets( vec3_t pos[], quaternion_t qua[], matrix3x4a_t *matrix, uint8_t *boneComputed );
    void SolveDependencies( vec3_t pos[], quaternion_t qua[], matrix3x4a_t*matrix, uint8_t *boneComputed );
    void ClearTargets( );
    void AddDependencies( void *seqdesc, int iSequence, float flCycle, const float poseParameters[], float flWeight );
    void CopyTo( CIKContext* other, const unsigned short *iRemapping );
};