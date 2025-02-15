#include "includes.h"

bool detours::Init( ) {
	if ( MH_Initialize( ) != MH_OK )
		throw std::runtime_error( XOR( "Unable to initialize minhook!" ) );

	const auto _paint = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 83 EC 40 53 8B D9 8B 0D ? ? ? ? 89" ) ).as< void* >( );
	const auto _packet_end = util::get_method < void* >( g_csgo.m_cl, CClientState::PACKETEND );
	const auto _should_skip_animation_frame = pattern::find( g_csgo.m_client_dll, XOR( "57 8B F9 8B 07 8B ? ? ? ? ? FF D0 84 C0 75 02" ) ).as < void* >( );
	const auto _check_for_sequence_change = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 51 53 8B 5D 08 56 8B F1 57 85" ) ).as < void* >( ); // 55 8B EC 51 53 8B 5D 08 56 8B F1 57 85
	const auto _standard_blending_rules = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6" ) ).as < void* >( ); //55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6
	const auto _modify_eye_position = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60" ) ).as < void* >( );
	const auto _base_interpolate_part1 = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 51 8B 45 14 56 8B F1 C7 00 ? ? ? ? 8B 06 8B 80" ) ).as < void* >( );
	const auto _animstate_update = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" ) ).as < void*  >( );
	const auto _do_procedural_footplant = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 83 EC 78 56 8B F1 57 8B 56 60 85" ) ).as < void* >( );
	const auto _setup_bones = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 8B 0D" ) ).as < void* >( );
	const auto _setup_movement = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F8 81 EC ? ? ? ? 56 57 8B 3D ? ? ? ? 8B" ) ).as < void* >( );
	const auto _update_client_side_animation = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74" ) ).as < void* >( );
	const auto _svcmsg_voicedata = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 83 E4 F8 A1 ? ? ? ? 81 EC ? ? ? ? 53 56 8B F1 B9 ? ? ? ? 57 FF 50 34 8B 7D 08 85 C0 74 13 8B 47 08 40 50 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 08 8B 47 08 89 44 24 1C 8D 48 01 8B 86 ? ? ? ? 40 89 4C 24 0C 3B C8 75 49" ) ).as < void* >( );
	const auto _process_movement = util::get_method < void* >( g_csgo.m_game_movement, CGameMovement::PROCESSMOVEMENT );
	const auto _get_eye_angles = pattern::find( g_csgo.m_client_dll, XOR( "56 8B F1 85 F6 74 32" ) ).as< void* >( );
	const auto _packet_start = pattern::find( g_csgo.m_engine_dll, "56 8B F1 E8 ? ? ? ? 8B 8E ? ? ? ? 3B" ).sub( 32 ).as< void* >( );
	const auto _cl_move = pattern::find( g_csgo.m_engine_dll, "55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A F9 F3 0F 11 45" ).as< void* >( );
	const auto _note_prediction_error = pattern::find( g_csgo.m_client_dll, "55 8B EC 83 EC 0C 56 8B F1 8B 06 8B 80 ? ? ? ? FF D0 84 C0 74 5B" ).as< void* >( );

	// create detours.
	MH_CreateHook( _cl_move, detours::CL_Move, ( void** )&old::CL_Move );
	MH_CreateHook( _paint, detours::Paint, ( void** )&old::Paint );
	MH_CreateHook( _packet_start, detours::PacketStart, ( void** )&old::PacketStart );
	MH_CreateHook( _should_skip_animation_frame, detours::ShouldSkipAnimationFrame, ( void** )&old::ShouldSkipAnimationFrame );
	MH_CreateHook( _check_for_sequence_change, detours::CheckForSequenceChange, ( void** )&old::CheckForSequenceChange );
	//MH_CreateHook( _standard_blending_rules, detours::StandardBlendingRules, ( void** )&old::StandardBlendingRules );
	MH_CreateHook( _get_eye_angles, detours::GetEyeAngles, ( void** )&old::GetEyeAngles );
	MH_CreateHook( _modify_eye_position, detours::ModifyEyePosition, ( void** )&old::ModifyEyePosition );
	MH_CreateHook( _base_interpolate_part1, detours::BaseInterpolatePart1, ( void** )&old::BaseInterpolatePart1 );
	MH_CreateHook( _animstate_update, detours::UpdateAnimationState, ( void** )&old::UpdateAnimationState );
	MH_CreateHook( _do_procedural_footplant, detours::DoProceduralFootPlant, ( void** )&old::DoProceduralFootPlant );
	MH_CreateHook( _setup_bones, detours::SetupBones, ( void** )&old::SetupBones );
	//MH_CreateHook( _setup_movement, detours::SetupMovement, ( void** )&old::SetupMovement );
	//MH_CreateHook ( _svcmsg_voicedata, detours::SVCMsg_VoiceData, ( void ** ) &old::SVCMsg_VoiceData );
	MH_CreateHook( _process_movement, detours::ProcessMovement, ( void** )&old::ProcessMovement );
	MH_CreateHook( _note_prediction_error, detours::NotePredictionError, ( void** )&old::NotePredictionError );

	// enable all hooks.
	MH_EnableHook( MH_ALL_HOOKS );

	return true;
}

void __fastcall detours::NotePredictionError( void *ecx, void *edx, const vec3_t& delta ) {
	return;
}

void __vectorcall detours::CL_Move( float accumulated_extra_samples, bool final_tick ) {
	if ( !g_csgo.m_cl || !g_csgo.m_engine->IsInGame( ) )
		return old::CL_Move( accumulated_extra_samples, final_tick );

	auto PacketReducer = [ & ]( ) -> void {
		static auto net_processsocket_fn = pattern::find( g_csgo.m_engine_dll, XOR( "55 8B EC 83 E4 ? 83 EC ? 53 56 8B D9 89 54 24 ? 57" ) ).as<void( __fastcall* )( int, uintptr_t )>( );

		if ( g_csgo.m_cl->m_nSignonState( ) == 6 && g_csgo.m_cl->m_net_channel && !g_csgo.m_engine->IsPaused( ) )
			net_processsocket_fn( 0, reinterpret_cast< uintptr_t >( g_csgo.m_cl ) + 12 );
		};

	const auto backup_globals = *g_csgo.m_globals;
	PacketReducer( );
	*g_csgo.m_globals = backup_globals;

	//if ( !globals::local_player || !globals::local_player->alive( ) ) {
		//globals::sent_commands.push_back( g_interfaces.client_state->last_outgoing_command( ) );
	 //   return original.call< void >( accumulated_extra_samples, final_tick );
	//}

	//const auto out_sequence = g_interfaces.client_state->last_outgoing_command( );

	old::CL_Move( accumulated_extra_samples, final_tick );

	//if ( out_sequence != g_interfaces.client_state->last_outgoing_command( ) && g_interfaces.client_state->net_channel )
		//globals::sent_commands.push_back( g_interfaces.client_state->last_outgoing_command( ) );
}

int __fastcall detours::BaseInterpolatePart1( void* ecx, void* edx, float& curtime, vec3_t& old_origin, ang_t& old_angs, int& no_more_changes ) {
	const auto player = reinterpret_cast< Player* >( ecx );

	if ( !player || !g_menu.main.aimbot.disable_vis_interp.get( ) )
		return old::BaseInterpolatePart1( ecx, edx, curtime, old_origin, old_angs, no_more_changes );

	static auto CBaseEntity__MoveToLastReceivedPos = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 80 7D 08 00 56 8B F1 75 0D 80 BE ? ? ? ? ? 0F" ) ).as < void( __thiscall* ) ( void*, bool ) >( );

	if ( player->IsPlayer( ) && player != g_cl.m_local ) {
		no_more_changes = 1;
		CBaseEntity__MoveToLastReceivedPos( player, false );
		return 0;
	}

	return old::BaseInterpolatePart1( ecx, edx, curtime, old_origin, old_angs, no_more_changes );
}

void __fastcall detours::ProcessMovement( void* ecx, void* edx, Entity* player, CMoveData* data ) {
	data->m_bGameCodeMovedPlayer = false;
	old::ProcessMovement( ecx, edx, player, data );
}

bool __fastcall detours::SVCMsg_VoiceData( void* ecx, void* edx, void* a2 ) {
	auto og = old::SVCMsg_VoiceData( ecx, edx, a2 );

	return og;
}

//void __fastcall detours::SetupMovement( void* ecx, void* edx ) {
//	auto state = ( CCSGOPlayerAnimState* )ecx;
//
//	if ( !state || !state->m_pPlayer || state->m_pPlayer != g_cl.m_local )
//		return old::SetupMovement( ecx, edx );
//
//	bool& m_bJumping = state->m_bFlashed;
//
//	if ( !( state->m_pPlayer->m_fFlags( ) & FL_ONGROUND )
//		&& state->m_bOnGround
//		&& state->m_pPlayer->m_vecVelocity( ).z > 0.0f ) {
//		m_bJumping = true;
//		rebuilt::SetSequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, rebuilt::SelectWeightedSequence( state, ACT_CSGO_JUMP ) );
//	}
//
//	old::SetupMovement( ecx, edx );
//
//	bool bPreviouslyOnLadder = state->m_bOnLadder;
//	state->m_bOnLadder = !state->m_bOnGround && state->m_pPlayer->m_MoveType( ) == MOVETYPE_LADDER;
//	bool bStartedLadderingThisFrame = ( !bPreviouslyOnLadder && state->m_bOnLadder );
//	bool bStoppedLadderingThisFrame = ( bPreviouslyOnLadder && !state->m_bOnLadder );
//
//	if ( state->m_bOnGround ) {
//		bool next_landing = false;
//
//		if ( !state->m_bLanding && ( state->m_bLandedOnGroundThisFrame || bStoppedLadderingThisFrame ) ) {
//			rebuilt::SetSequence( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, rebuilt::SelectWeightedSequence( state, ( state->m_flDurationInAir > 1 ) ? ACT_CSGO_LAND_HEAVY : ACT_CSGO_LAND_LIGHT ) );
//			//rebuilt::SetCycle ( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0 );
//			next_landing = true;
//		}
//
//		state->m_flDurationInAir = 0;
//
//		if ( next_landing && rebuilt::GetLayerActivity( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) != ACT_CSGO_CLIMB_LADDER ) {
//			m_bJumping = false;
//
//			rebuilt::IncrementLayerCycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, false );
//			rebuilt::IncrementLayerCycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );
//
//			state->m_pPlayer->m_flPoseParameter( )[ POSE_JUMP_FALL ] = 0.f;
//
//			if ( rebuilt::IsLayerSequenceCompleted( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) ) {
//				state->m_bLanding = false;
//				rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0 );
//				rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, 0 );
//				state->m_flLandAnimMultiplier = 1.0f;
//			}
//			else {
//				float flLandWeight = rebuilt::GetLayerIdealWeightFromSequenceCycle( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ) * state->m_flLandAnimMultiplier;
//				flLandWeight *= std::clamp< float >( ( 1.0f - state->m_flAnimDuckAmount ), 0.2f, 1.0f );
//
//				rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, flLandWeight );
//
//				float flCurrentJumpFallWeight = state->m_pPlayer->m_AnimOverlay( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_weight;
//				if ( flCurrentJumpFallWeight > 0 ) {
//					flCurrentJumpFallWeight = valve_math::Approach( 0, flCurrentJumpFallWeight, state->m_flLastUpdateIncrement * 10.0f );
//					rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, flCurrentJumpFallWeight );
//				}
//			}
//		}
//
//		if ( !state->m_bLanding && !m_bJumping && state->m_flLadderWeight <= 0 ) {
//			rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0 );
//		}
//	}
//	else if ( !state->m_bOnLadder ) {
//		state->m_bLanding = false;
//
//		// we're in the air
//		if ( state->m_bLeftTheGroundThisFrame || bStoppedLadderingThisFrame ) {
//			// If entered the air by jumping, then we already set the jump activity.
//			// But if we're in the air because we strolled off a ledge or the floor collapsed or something,
//			// we need to set the fall activity here.
//			if ( !m_bJumping ) {
//				rebuilt::SetSequence( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, rebuilt::SelectWeightedSequence( state, ACT_CSGO_FALL ) );
//			}
//
//			state->m_flDurationInAir = 0;
//		}
//
//		state->m_flDurationInAir += state->m_flLastUpdateIncrement;
//
//		rebuilt::IncrementLayerCycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false );
//
//		// increase jump weight
//		float flJumpWeight = state->m_pPlayer->m_AnimOverlay( )[ ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL ].m_weight;
//		float flNextJumpWeight = rebuilt::GetLayerIdealWeightFromSequenceCycle( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL );
//
//		if ( flNextJumpWeight > flJumpWeight )
//			rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, flNextJumpWeight );
//
//		auto smoothstep_bounds = [ ]( float edge0, float edge1, float x ) {
//			x = std::clamp< float >( ( x - edge0 ) / ( edge1 - edge0 ), 0, 1 );
//			return x * x * ( 3 - 2 * x );
//			};
//
//		float flLingeringLandWeight = state->m_pPlayer->m_AnimOverlay( )[ ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB ].m_weight;
//
//		if ( flLingeringLandWeight > 0 ) {
//			flLingeringLandWeight *= smoothstep_bounds( 0.2f, 0.0f, state->m_flDurationInAir );
//			rebuilt::SetWeight( state, ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, flLingeringLandWeight );
//		}
//
//		// blend jump into fall. This is a no-op if we're playing a fall anim.
//		state->m_pPlayer->m_flPoseParameter( )[ POSE_JUMP_FALL ] = std::clamp( smoothstep_bounds( 0.72f, 1.52f, state->m_flDurationInAir ), 0.0f, 1.0f );
//	}
//}

void __vectorcall detours::UpdateAnimationState( void* ecx, void* a1, float a2, float a3, float a4, void* a5 ) {
	const auto state = ( CCSGOPlayerAnimState* )ecx;

	if ( state->m_pPlayer != g_cl.m_local )
		return old::UpdateAnimationState( ecx, a1, a2, a3, a4, a5 );

	auto angle = g_cl.m_angle;

	return old::UpdateAnimationState( ecx, a1, angle.z, angle.y, angle.x, a5 );
}

void __fastcall detours::DoProceduralFootPlant( void* ecx, void* edx, int a1, int a2, int a3, int a4 ) {
	if ( ( Player* )ecx != g_cl.m_local )
		return old::DoProceduralFootPlant( ecx, edx, a1, a2, a3, a4 );

	return;
}

ang_t& __fastcall detours::GetEyeAngles( void* ecx, void* edx ) {
	if ( !g_cl.m_processing || !ecx || ecx != g_cl.m_local )
		return old::GetEyeAngles( ecx, edx );

	static auto ret_to_thirdperson_pitch = pattern::find( g_csgo.m_client_dll, XOR( "8B CE F3 0F 10 00 8B 06 F3 0F 11 45 ? FF 90 ? ? ? ? F3 0F 10 55" ) ).as< void* >( );
	static auto ret_to_thirdperson_yaw = pattern::find( g_csgo.m_client_dll, XOR( "F3 0F 10 55 ? 51 8B 8E" ) ).as< void* >( );

	if ( _ReturnAddress( ) == ret_to_thirdperson_pitch || _ReturnAddress( ) == ret_to_thirdperson_yaw )
		return g_cl.m_angle;

	return old::GetEyeAngles( ecx, edx );
}

bool __fastcall detours::SetupBones( void* ecx, void* edx, BoneArray* out, int max, int mask, float curtime ) {
	auto base_entity = reinterpret_cast< Player* >( reinterpret_cast< uintptr_t >( ecx ) - 4 );

	if ( !g_cl.m_processing || !base_entity || !base_entity->alive( ) )
		return old::SetupBones( ecx, edx, out, max, mask, curtime );

	if ( !g_cl.m_allow_bones[ base_entity->index( ) ] && base_entity->IsPlayer( ) ) {
		static auto& g_model_bone_counter = *pattern::find( g_csgo.m_client_dll, XOR( "3B 05 ? ? ? ? 0F 84 ? ? ? ? 8B 47" ) ).add( 2 ).to( ).as< int* >( );

		auto base_animating = reinterpret_cast< Player* >( ecx );
		auto owner = base_entity->GetRootMoveParent( );
		auto main_entity = owner ? owner : base_entity;

		if ( *reinterpret_cast< int* >( reinterpret_cast< uintptr_t >( main_entity ) + 0x2680 ) != g_model_bone_counter ) {
			memcpy( base_animating->m_pBoneCache( ), g_cl.m_usable_bones[ base_entity->index( ) ].data( ), sizeof( BoneArray ) * base_animating->m_iBoneCount( ) );

			/* apply animated bone origin. */
			for ( auto i = 0; i < base_animating->m_iBoneCount( ); i++ )
				base_animating->m_pBoneCache( )[ i ].SetOrigin( base_animating->m_pBoneCache( )[ i ].GetOrigin( ) - g_cl.m_usable_origin[ base_entity->index( ) ] + base_entity->GetRenderOrigin( ) );

			static auto attachment_helper = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4 56 57 85 DB 0F 84" ) ).as< void( __thiscall* )( void*, void* ) >( );

			if ( base_entity->GetCStudioHdr( ) )
				attachment_helper( base_entity, base_entity->GetCStudioHdr( ) );

			*reinterpret_cast< int* >( reinterpret_cast< uintptr_t >( main_entity ) + 0x2680 ) = g_model_bone_counter;
		}

		if ( out ) {
			if ( max >= base_animating->m_iBoneCount( ) )
				memcpy( out, base_animating->m_pBoneCache( ), sizeof( BoneArray ) * base_animating->m_iBoneCount( ) );
			else
				return false;
		}

		return true;
	}

	return old::SetupBones( ecx, edx, out, max, mask, curtime );
}

void __fastcall detours::Paint( void* ecx, void* edx, PaintModes_t mode ) {
	old::Paint( ecx, edx, mode );

	const auto vgui = reinterpret_cast< IEngineVGui* >( ecx );

	static auto StartDrawing = pattern::find( g_csgo.m_surface_dll, XOR( "55 8B EC 83 E4 C0 83 EC 38 80 3D ? ? ? ? ? 56 57 8B F9 75 53" ) ).as< void( __thiscall* )( void* ) >( );
	static auto FinishDrawing = pattern::find( g_csgo.m_surface_dll, XOR( "8B 0D ? ? ? ? 56 C6 05 ? ? ? ? ? 8B 01 FF 90" ) ).as<  void( __thiscall* )( void* ) >( );

	if ( vgui->m_static_transition_panel && ( mode & PAINT_UIPANELS ) ) {
		StartDrawing( g_csgo.m_surface );
		g_hvh.update_manual_direction( );
		g_cl.OnPaint( );
		FinishDrawing( g_csgo.m_surface );
	}
}

void __fastcall detours::ModifyEyePosition( void* ecx, void* edx, vec3_t& eye_pos ) {
	auto state = reinterpret_cast < CCSGOPlayerAnimState* > ( ecx );

	if ( !state )
		return old::ModifyEyePosition( ecx, edx, eye_pos );

	*( bool* )( std::uintptr_t( state ) + 0x328 ) = false;

	if ( state->m_pPlayer == g_cl.m_local )
		return state->m_pPlayer->ModifyEyePosition( state, &eye_pos );

	return old::ModifyEyePosition( ecx, edx, eye_pos );
}

int __fastcall detours::PacketStart( void* ecx, void* edx, int incoming_sequence, int outgoing_acknowledged ) {
	if ( !g_csgo.m_cl || !g_csgo.m_engine->IsInGame( ) )
		return old::PacketStart( ecx, edx, incoming_sequence, outgoing_acknowledged );

	if ( g_cl.m_outgoing_cmds[ outgoing_acknowledged % 150 ].cmd == outgoing_acknowledged )
		old::PacketStart( ecx, edx, incoming_sequence, outgoing_acknowledged );
}

bool __fastcall detours::ShouldSkipAnimationFrame( void* ecx, void* edx ) {
	return false;
}

void __fastcall detours::CheckForSequenceChange( void* ecx, void* edx, void* hdr, int sequence, bool force_new_sequence, bool interpolate ) {
	old::CheckForSequenceChange( ecx, edx, hdr, sequence, force_new_sequence, false );
}

void __fastcall detours::StandardBlendingRules( void* ecx, void* edx, void* hdr, void* pos, void* q, float current_time, int bone_mask ) {
	//const auto player = reinterpret_cast< Player* >( ecx );

	//if ( !player || player != g_cl.m_local )
	//	return old::StandardBlendingRules( ecx, edx, hdr, pos, q, current_time, bone_mask );

	//player->m_fEffects( ) |= EF_NOINTERP;

	old::StandardBlendingRules( ecx, edx, hdr, pos, q, current_time, bone_mask );

	//player->m_fEffects( ) &= ~EF_NOINTERP;
}