#include "includes.h"

CIKContext::CIKContext( ) {
    using ConstructFn = CIKContext *( __thiscall * ) ( CIKContext * );
    static auto Construct = pattern::find( g_csgo.m_client_dll, XOR( "56 8B F1 6A 00 6A 00 C7 86 ? ? ? ? ? ? ? ? 89 B6 ? ? ? ? C7 86" ) ).as< ConstructFn >( );
    Construct( this );
}

void CIKContext::Init( const CStudioHdr *hdr, const ang_t &local_angles, const vec3_t &local_origin, float current_time, int frame_count, int bone_mask ) {
    using InitFn = void( __thiscall * )( void *, const CStudioHdr *, const ang_t&, const vec3_t &, float, int, int );
    static auto Init = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F ? ? ? ? 89 7D" ) ).as< InitFn >( );
    Init( this, hdr, local_angles, local_origin, current_time, frame_count, bone_mask );
}

void CIKContext::UpdateTargets( vec3_t pos[], quaternion_t qua[], matrix3x4a_t *matrix, uint8_t *boneComputed ) {
    using UpdateTargetsFn = void( __thiscall * )( void *, vec3_t [], quaternion_t [], matrix3x4a_t *, uint8_t * );
    static auto UpdateTargets = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 81 ? ? ? ? ? 33 D2 89" ) ).as< UpdateTargetsFn >( );
    UpdateTargets( this, pos, qua, matrix, boneComputed );
}

void CIKContext::SolveDependencies( vec3_t pos[], quaternion_t qua[], matrix3x4a_t *matrix, uint8_t *boneComputed ) {
    using SolveDependenciesFn = void( __thiscall * )( void *, vec3_t[], quaternion_t[], matrix3x4a_t *, uint8_t * );
    static auto SolveDependencies = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 81 EC ? ? ? ? 8B 81" ) ).as< SolveDependenciesFn >( );
    SolveDependencies( this, pos, qua, matrix, boneComputed );
}

void CIKContext::ClearTargets( ) {
    int v49 = 0;

    if ( *( int * ) ( std::uintptr_t( this ) + 4080 ) > 0 ) {
        int *iFramecounter = ( int * ) ( std::uintptr_t( this ) + 0xD0 );
        do {
            *iFramecounter = -9999;
            iFramecounter += 85;
            ++v49;
        } while ( v49 < *( int * ) ( std::uintptr_t( this ) + 0xFF0 ) );
    }
}

void CIKContext::AddDependencies( void *seqdesc, int iSequence, float flCycle, const float poseParameters[], float flWeight ) {
    using AddDependenciesFn = void( __thiscall * )( void *, void *, int, float, const float[], float );
    static auto AddDependencies = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 81 EC ? ? ? ? 53 56 57 8B F9 0F 28 CB" ) ).as< uintptr_t >( );

    __asm
    {
			mov ecx, this
			movss xmm3, flCycle
			push flWeight
			push poseParameters
			push iSequence
			push seqdesc
			call AddDependencies
    }
}

void CIKContext::CopyTo( CIKContext *other, const unsigned short *iRemapping ) {
    using CopyToFn = void( __thiscall * )( void *, CIKContext *, const unsigned short * );
    static auto CopyTo = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC ? 8B 45 ? 57 8B F9 89 7D ?") ).as< CopyToFn >( );
    CopyTo( this, other, iRemapping );
}