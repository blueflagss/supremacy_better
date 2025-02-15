#include "includes.h"
// from my hake
bone_setup g_bone_setup = { };

#pragma optimize(off, "")
void bone_setup::build( Player* pl, BoneArray* bone_to_world, int mask, const vec3_t& origin, const ang_t& angles, float time, const ang_t& eye_angles ) {
	if ( !pl )
		return;
	
	auto cstudio_hdr = pl->GetCStudioHdr( );

	if ( !cstudio_hdr )
		return;

	auto accessor = &pl->m_BoneAccessor( );

	if ( !accessor )
		return;

	const auto backup_matrix = accessor->m_pBones;

	if ( !backup_matrix )
		return;

	g_cl.m_allow_bones[ pl->index( ) ] = true;

	std::array< C_AnimationLayer, 13 > backup_layers;
	std::memcpy( backup_layers.data( ), pl->m_AnimOverlay( ), sizeof( backup_layers ) );

	matrix3x4a_t tmp[ 128 ];
	accessor->m_pBones = tmp;

	mask |= BONE_ALWAYS_SETUP;
	mask &= ~( BONE_USED_BY_BONE_MERGE );

	pl->m_AnimOverlay( )[ 12 ].m_weight = 0.f;

	uint8_t bone_computed[ 0x100 ] = { 0 };

	alignas( 16 ) vec3_t pos[ 128 ];
	alignas( 16 ) quaternion_t q[ 128 ];

	ZeroMemory( pos, sizeof( vec3_t[ 128 ] ) );
	ZeroMemory( q, sizeof( quaternion_t[ 128 ] ) );

	const auto backup_bone_setup_time = pl->m_flLastBoneSetupTime( );
	const auto backup_effects = pl->m_fEffects( );
	const auto backup_ik = pl->m_pIk( );
	const auto backup_abs_angles = pl->GetAbsAngles( );
	const auto backup_eye_angle = pl->m_angEyeAngles( );
	const auto backup_view_offset = pl->m_vecViewOffset( );
	const auto backup_lod_flags = pl->m_iAnimLODFlags( );
	const auto backup_time = g_csgo.m_globals->m_curtime;

	pl->m_fEffects( ) |= EF_NOINTERP;
	pl->m_bIsJiggleBonesEnabled( ) = false;
	pl->m_iAnimLODFlags( ) = 0;
	pl->m_angEyeAngles( ) = eye_angles;
	pl->m_flLastBoneSetupTime( ) = 0;

	g_csgo.m_globals->m_curtime = time;

	CIKContext ik;
	ik.ClearTargets( );
	ik.Init( cstudio_hdr, angles, origin, time, g_csgo.m_globals->m_frame, mask );

	pl->m_pIk( ) = &ik;

	pl->StandardBlendingRules( cstudio_hdr, pos, q, time, mask );
	pl->UpdateIKLocks( time );
	ik.UpdateTargets( pos, q, accessor->m_pBones, bone_computed );
	pl->CalculateIKLocks( time );
	ik.SolveDependencies( pos, q, accessor->m_pBones, bone_computed );

	matrix3x4a_t rot_matrix;
	math::AngleMatrix( angles, origin, rot_matrix );
	rot_matrix.SetOrigin( origin );

	pl->BuildTransformations( cstudio_hdr, pos, q, rot_matrix, mask, bone_computed );
	std::memcpy( bone_to_world, accessor->m_pBones, sizeof( BoneArray ) * 128 );
	accessor->m_pBones = backup_matrix;

	pl->m_iEFlags( ) &= ~0x003;
	pl->SetAbsAngles( backup_abs_angles );
	pl->m_angEyeAngles( ) = backup_eye_angle;
	pl->m_fEffects( ) = backup_effects;
	pl->m_pIk( ) = backup_ik;
	pl->m_vecViewOffset( ) = backup_view_offset;
	pl->m_iAnimLODFlags( ) = backup_lod_flags;
	pl->m_flLastBoneSetupTime( ) = backup_bone_setup_time;

	g_csgo.m_globals->m_curtime = backup_time;

	std::memcpy( pl->m_AnimOverlay( ), backup_layers.data( ), sizeof( backup_layers ) );
	g_cl.m_allow_bones[ pl->index( ) ] = false;
}
#pragma optimize(on, "")