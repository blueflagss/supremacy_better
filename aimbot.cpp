#include "includes.h"
#include "threading/threading.h"

Aimbot g_aimbot{ };;

void AimPlayer::UpdateAnimations( LagRecord* record ) {
	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState( );
	if ( !state )
		return;

	if ( m_player->m_flSpawnTime( ) != m_spawn ) {
		game::ResetAnimationState( state );

		m_spawn = m_player->m_flSpawnTime( );
	}

	float curtime = g_csgo.m_globals->m_curtime;
	float frametime = g_csgo.m_globals->m_frametime;

	const auto dword_44732EA4 = g_csgo.m_globals->m_realtime;
	const auto dword_44732EA8 = g_csgo.m_globals->m_curtime;
	const auto dword_44732EAC = g_csgo.m_globals->m_frametime;
	const auto dword_44732EB0 = g_csgo.m_globals->m_abs_frametime;
	const auto dword_44732EB4 = g_csgo.m_globals->m_interp_amt;
	const auto dword_44732EB8 = g_csgo.m_globals->m_frame;
	const auto dword_44732EBC = g_csgo.m_globals->m_tick_count;

	g_csgo.m_globals->m_realtime = g_csgo.m_globals->m_curtime;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_abs_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frame = record->m_anim_time;
	g_csgo.m_globals->m_tick_count = record->m_anim_time;
	g_csgo.m_globals->m_interp_amt = 0.0f;

	AnimationBackup_t backup;

	backup.m_origin = m_player->m_vecOrigin( );
	backup.m_abs_origin = m_player->GetAbsOrigin( );
	backup.m_velocity = m_player->m_vecVelocity( );
	backup.m_abs_velocity = m_player->m_vecAbsVelocity( );
	backup.m_flags = m_player->m_fFlags( );
	backup.m_eflags = m_player->m_iEFlags( );
	backup.m_duck = m_player->m_flDuckAmount( );
	backup.m_body = m_player->m_flLowerBodyYawTarget( );
	backup.m_simtime = m_player->m_flSimulationTime( );

	m_player->GetAnimLayers( backup.m_layers );

	bool bot = game::IsFakePlayer( m_player->index( ) );

	record->m_fake_walk = false;
	record->m_mode = Resolver::Modes::RESOLVE_NONE;

	if ( record->m_lag > 0 && record->m_lag < 16 && m_records.size( ) >= 2 ) {
		LagRecord* previous = m_records[ 1 ].get( );

		if ( previous && !previous->dormant( ) )
			record->m_velocity = ( record->m_origin - previous->m_origin ) * ( 1.f / game::TICKS_TO_TIME( record->m_lag ) );
	}

	record->m_anim_velocity = m_player->m_vecVelocity( );

	if ( record->m_lag > 1 && !bot ) {
		float speed = m_player->m_PlayerAnimState( )->m_vecVelocity.length( );

		if ( record->m_flags & FL_ONGROUND && record->m_layers[ 6 ].m_weight == 0.f && speed > 0.1f && speed < 100.f )
			record->m_fake_walk = true;

		if ( record->m_fake_walk )
			record->m_anim_velocity = m_player->m_vecVelocity( ) = { 0.f, 0.f, 0.f };

		if ( m_records.size( ) >= 2 ) {
			LagRecord* previous = m_records[ 1 ].get( );

			if ( previous && !previous->dormant( ) ) {
				m_player->m_fFlags( ) = previous->m_flags;

				m_player->m_fFlags( ) &= ~FL_ONGROUND;

				if ( record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND )
					m_player->m_fFlags( ) |= FL_ONGROUND;

				if ( record->m_layers[ 4 ].m_weight != 1.f && previous->m_layers[ 4 ].m_weight == 1.f && record->m_layers[ 5 ].m_weight != 0.f )
					m_player->m_fFlags( ) |= FL_ONGROUND;

				if ( record->m_flags & FL_ONGROUND && !( previous->m_flags & FL_ONGROUND ) )
					m_player->m_fFlags( ) &= ~FL_ONGROUND;

				float time = record->m_sim_time - previous->m_sim_time;

				if ( !record->m_fake_walk ) {
					vec3_t velo = record->m_velocity - previous->m_velocity;

					vec3_t accel = ( velo / time ) * g_csgo.m_globals->m_interval;

					record->m_anim_velocity = previous->m_velocity + accel;
				}
			}
		}
	}

	bool fake = !bot && g_menu.main.aimbot.correct.get( );

	if ( fake )
		g_resolver.ResolveAngles( m_player, record );

	m_player->m_vecOrigin( ) = record->m_origin;
	m_player->m_vecVelocity( ) = backup.m_velocity;
	m_player->m_vecAbsVelocity( ) = record->m_anim_velocity;
	m_player->m_flLowerBodyYawTarget( ) = record->m_body;
	m_player->m_iEFlags( ) &= ~0x1000;
	m_player->m_angEyeAngles( ) = record->m_eye_angles;

	if ( state->m_nLastUpdateFrame == g_csgo.m_globals->m_frame )
		state->m_nLastUpdateFrame = g_csgo.m_globals->m_frame - 1;

	if ( state->m_flLastUpdateTime == g_csgo.m_globals->m_curtime )
		state->m_flLastUpdateTime = g_csgo.m_globals->m_curtime - g_csgo.m_globals->m_interval;

	m_player->m_bClientSideAnimation( ) = true;
	m_player->UpdateClientSideAnimation( );
	m_player->m_bClientSideAnimation( ) = false;

	if ( fake )
		g_resolver.ResolvePoses( m_player, record );

	m_player->GetPoseParameters( record->m_poses );
	record->m_abs_ang = m_player->GetAbsAngles( );

	m_player->m_vecOrigin( ) = backup.m_origin;
	m_player->m_vecVelocity( ) = backup.m_velocity;
	m_player->m_vecAbsVelocity( ) = backup.m_abs_velocity;
	m_player->m_fFlags( ) = backup.m_flags;
	m_player->m_iEFlags( ) = backup.m_eflags;
	m_player->m_flDuckAmount( ) = backup.m_duck;
	m_player->m_flLowerBodyYawTarget( ) = backup.m_body;
	m_player->SetAbsOrigin( backup.m_origin );
	m_player->SetAnimLayers( backup.m_layers );
	m_player->m_flSimulationTime( ) = backup.m_simtime;

	g_csgo.m_globals->m_realtime = dword_44732EA4;
	g_csgo.m_globals->m_curtime = dword_44732EA8;
	g_csgo.m_globals->m_frametime = dword_44732EAC;
	g_csgo.m_globals->m_abs_frametime = dword_44732EB0;
	g_csgo.m_globals->m_interp_amt = dword_44732EB4;
	g_csgo.m_globals->m_frame = dword_44732EB8;
	g_csgo.m_globals->m_tick_count = dword_44732EBC;
}

void AimPlayer::OnNetUpdate( Player* player ) {
	bool post_update = ( player->m_lifeState( ) == LIFE_DEAD );
	bool disable = ( !post_update && !g_cl.m_processing );

	if ( post_update ) {
		player->m_bClientSideAnimation( ) = true;
		m_records.clear( );
		return;
	}

	if ( disable ) {
		player->m_bClientSideAnimation( ) = true;
		return;
	}

	if ( m_player != player )
		m_records.clear( );

	m_player = player;

	if ( player->dormant( ) ) {
		bool insert = true;

		if ( !m_records.empty( ) ) {
			LagRecord* front = m_records.front( ).get( );

			if ( front->dormant( ) )
				insert = false;
		}

		if ( insert ) {
			m_records.emplace_front( std::make_shared< LagRecord >( player ) );

			LagRecord* current = m_records.front( ).get( );

			current->m_dormant = true;
		}
	}

	bool update = ( m_records.empty( ) || player->m_flSimulationTime( ) != player->m_flOldSimulationTime( ) );

	if ( update ) {
		m_records.emplace_front( std::make_shared< LagRecord >( player ) );

		LagRecord* current = m_records.front( ).get( );
		current->m_dormant = false;

		UpdateAnimations( current );

		g_bone_setup.build( m_player, current->m_bones.data( ), BONE_USED_BY_ANYTHING, current->m_origin, current->m_abs_ang, current->m_sim_time, current->m_eye_angles );
	}

	while ( m_records.size( ) > 256 )
		m_records.pop_back( );
}

void AimPlayer::OnRoundStart( Player* player ) {
	m_player = player;
	m_walk_record = LagRecord{ };
	m_shots = 0;
	m_missed_shots = 0;

	m_stand_index = 0;
	m_stand_index2 = 0;
	m_body_index = 0;

	m_records.clear( );
	m_hitboxes.clear( );
}

void AimPlayer::SetupHitboxes( LagRecord* record, bool history ) {
	m_hitboxes.clear( );

	if ( g_cl.m_weapon_id == ZEUS ) {
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
		return;
	}

	if ( g_menu.main.aimbot.baim1.get( 0 ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	if ( g_menu.main.aimbot.baim1.get( 1 ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::LETHAL } );

	if ( g_menu.main.aimbot.baim1.get( 2 ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::LETHAL2 } );

	if ( g_menu.main.aimbot.baim1.get( 3 ) && record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	if ( g_menu.main.aimbot.baim1.get( 4 ) && !( record->m_pred_flags & FL_ONGROUND ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	bool only{ false };

	if ( g_menu.main.aimbot.baim2.get( 0 ) ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	if ( g_menu.main.aimbot.baim2.get( 1 ) && m_player->m_iHealth( ) <= ( int )g_menu.main.aimbot.baim_hp.get( ) ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	if ( g_menu.main.aimbot.baim2.get( 2 ) && record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	if ( g_menu.main.aimbot.baim2.get( 3 ) && !( record->m_pred_flags & FL_ONGROUND ) ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	if ( g_input.GetKeyState( g_menu.main.aimbot.baim_key.get( ) ) ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	if ( only )
		return;

	std::vector< size_t > hitbox{ history ? g_menu.main.aimbot.hitbox_history.GetActiveIndices( ) : g_menu.main.aimbot.hitbox.GetActiveIndices( ) };
	if ( hitbox.empty( ) )
		return;

	for ( const auto& h : hitbox ) {
		if ( h == 0 )
			m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::NORMAL } );

		if ( h == 1 ) {
			m_hitboxes.push_back( { HITBOX_THORAX, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_CHEST, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_UPPER_CHEST, HitscanMode::NORMAL } );
		}

		if ( h == 2 ) {
			m_hitboxes.push_back( { HITBOX_PELVIS, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::NORMAL } );
		}

		if ( h == 3 ) {
			m_hitboxes.push_back( { HITBOX_L_UPPER_ARM, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_R_UPPER_ARM, HitscanMode::NORMAL } );
		}

		if ( h == 4 ) {
			m_hitboxes.push_back( { HITBOX_L_THIGH, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_R_THIGH, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_L_CALF, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_R_CALF, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_L_FOOT, HitscanMode::NORMAL } );
			m_hitboxes.push_back( { HITBOX_R_FOOT, HitscanMode::NORMAL } );
		}
	}
}

void Aimbot::Init( ) {
	m_targets.clear( );

	m_target = nullptr;
	m_aim = vec3_t{ };
	m_angle = ang_t{ };
	m_damage = 0.f;
	m_record = nullptr;
	m_stop = false;

	m_best_dist = std::numeric_limits< float >::max( );
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = std::numeric_limits< float >::max( );
	m_best_height = std::numeric_limits< float >::max( );
}

void Aimbot::StripAttack( ) {
	if ( g_cl.m_weapon_id == REVOLVER )
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK2;

	else
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
}

void Aimbot::think( ) {
	Init( );

	if ( !g_cl.m_weapon )
		return;

	if ( g_cl.m_weapon_type == WEAPONTYPE_GRENADE || g_cl.m_weapon_type == WEAPONTYPE_C4 )
		return;

	if ( !g_menu.main.aimbot.enable.get( ) )
		return;

	bool revolver = g_cl.m_weapon_id == REVOLVER && g_cl.m_revolver_cock != 0;

	if ( revolver && g_cl.m_revolver_cock > 0 && g_cl.m_revolver_cock == g_cl.m_revolver_query ) {
		*g_cl.m_packet = false;
		return;
	}

	if ( !g_cl.m_weapon_fire )
		return;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		if ( !IsValidTarget( player ) )
			continue;

		AimPlayer* data = &m_players[ i - 1 ];
		if ( !data )
			continue;

		m_targets.emplace_back( data );
	}

	if ( g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS ) {
		if ( g_menu.main.aimbot.knifebot.get( ) )
			knife( );

		return;
	}

	find( );
	update( );
}

void Aimbot::find( ) {
	struct BestTarget_t { Player* player; int hitbox;  vec3_t pos; float damage; LagRecord* record; };

	vec3_t       tmp_pos;
	float        tmp_damage;
	int			 tmp_hitbox;
	BestTarget_t best;
	best.player = nullptr;
	best.damage = -1.f;
	best.hitbox = HITBOX_HEAD;
	best.pos = vec3_t{ };
	best.record = nullptr;

	if ( m_targets.empty( ) )
		return;

	if ( g_cl.m_weapon_id == ZEUS && !g_menu.main.aimbot.zeusbot.get( ) )
		return;

	for ( const auto& t : m_targets ) {
		if ( t->m_records.empty( ) )
			continue;

		if ( g_lagcomp.StartPrediction( t ) ) {
			LagRecord* front = t->m_records.front( ).get( );

			t->SetupHitboxes( front, false );
			if ( t->m_hitboxes.empty( ) )
				continue;

			if ( t->GetBestAimPosition( tmp_pos, tmp_hitbox, tmp_damage, front ) && SelectTarget( front, tmp_pos, tmp_damage ) ) {
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = front;
				best.hitbox = tmp_hitbox;
			}
		}

		else {
			if ( t->m_delay_shot && g_menu.main.aimbot.delay_shot.get( ) )
				continue;

			LagRecord* ideal = g_resolver.FindIdealRecord( t );
			if ( !ideal )
				continue;

			t->SetupHitboxes( ideal, false );
			if ( t->m_hitboxes.empty( ) )
				continue;

			if ( t->GetBestAimPosition( tmp_pos, tmp_hitbox, tmp_damage, ideal ) && SelectTarget( ideal, tmp_pos, tmp_damage ) ) {
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = ideal;
				best.hitbox = tmp_hitbox;
			}

			LagRecord* last = g_resolver.FindLastRecord( t );
			if ( !last || last == ideal )
				continue;

			t->SetupHitboxes( last, true );
			if ( t->m_hitboxes.empty( ) )
				continue;

			if ( t->GetBestAimPosition( tmp_pos, tmp_hitbox, tmp_damage, last ) && SelectTarget( last, tmp_pos, tmp_damage ) ) {
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.hitbox = tmp_hitbox;
				best.record = last;
			}
		}
	}

	if ( best.damage > 0 && !g_input.GetKeyState( g_menu.main.movement.fakewalk.get( ) ) )
		Slow( g_cl.m_cmd );

	if ( best.player && best.record ) {
		const auto backup_origin = best.player->m_vecOrigin( );
		const auto backup_mins = best.player->m_vecMins( );
		const auto backup_maxs = best.player->m_vecMins( );
		const auto backup_bone_cache = best.player->m_pBoneCache( );

		math::VectorAngles( best.pos - g_cl.m_shoot_pos, m_angle );

		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_record = best.record;

		m_record->cache( );

		bool on = g_menu.main.aimbot.hitchance.get( ) && g_menu.main.config.mode.get( ) == 0;
		bool hit = on && CheckHitchance( m_target, m_angle );

		bool can_scope = !g_cl.m_local->m_bIsScoped( ) && ( g_cl.m_weapon_id == AUG || g_cl.m_weapon_id == SG553 || g_cl.m_weapon_type == WEAPONTYPE_SNIPER_RIFLE );

		if ( can_scope ) {
			if ( g_menu.main.aimbot.zoom.get( ) == 1 ) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}

			else if ( g_menu.main.aimbot.zoom.get( ) == 2 && on && !hit ) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}
		}

		if ( hit || !on ) {
			if ( g_menu.main.config.mode.get( ) == 1 && g_cl.m_weapon_id == REVOLVER )
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;

			else
				g_cl.m_cmd->m_buttons |= IN_ATTACK;
		}

		best.player->m_vecOrigin( ) = backup_origin;
		best.player->m_vecMins( ) = backup_mins;
		best.player->m_vecMaxs( ) = backup_maxs;
		best.player->m_pBoneCache( ) = backup_bone_cache;
	}
}


bool Aimbot::CheckHitchance( Player* player, const ang_t& angle ) {
	constexpr float HITCHANCE_MAX = 100.f;
	constexpr int   SEED_MAX = 255;

	vec3_t     start{ g_cl.m_shoot_pos }, end, fwd, right, up, dir, wep_spread;
	float      inaccuracy, spread;
	CGameTrace tr;
	size_t     total_hits{ }, needed_hits{ ( size_t )std::ceil( ( g_menu.main.aimbot.hitchance_amount.get( ) * SEED_MAX ) / HITCHANCE_MAX ) };

	// get needed directional vectors.
	math::AngleVectors( angle, &fwd, &right, &up );

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
	inaccuracy = g_cl.m_weapon->GetInaccuracy( );
	spread = g_cl.m_weapon->GetSpread( );

	// iterate all possible seeds.
	for ( int i{ }; i <= SEED_MAX; ++i ) {
		// get spread.
		wep_spread = g_cl.m_weapon->CalculateSpread( i, inaccuracy, spread );

		// get spread direction.
		dir = ( fwd + ( right * wep_spread.x ) + ( up * wep_spread.y ) ).normalized( );

		// get end of trace.
		end = start + ( dir * g_cl.m_weapon_info->m_range );

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT, player, &tr );

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if ( tr.m_entity == player && game::IsValidHitgroup( tr.m_hitgroup ) )
			++total_hits;

		// we made it.
		if ( total_hits >= needed_hits )
			return true;

		// we cant make it anymore.
		if ( ( SEED_MAX - i + total_hits ) < needed_hits )
			return false;
	}

	return false;
}

bool AimPlayer::SetupHitboxPoints( LagRecord* record, BoneArray* bones, int index, std::vector< vec3_t >& points ) {
	points.clear( );

	const model_t* model = m_player->GetModel( );
	if ( !model )
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet( m_player->m_nHitboxSet( ) );
	if ( !set )
		return false;

	mstudiobbox_t* bbox = set->GetHitbox( index );
	if ( !bbox )
		return false;

	float scale = g_menu.main.aimbot.scale.get( ) / 100.f;

	if ( !( record->m_pred_flags ) & FL_ONGROUND )
		scale = 0.7f;

	float bscale = g_menu.main.aimbot.body_scale.get( ) / 100.f;

	if ( bbox->m_radius <= 0.f ) {
		matrix3x4_t rot_matrix;
		g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

		matrix3x4_t matrix;
		math::ConcatTransforms( bones[ bbox->m_bone ], rot_matrix, matrix );

		vec3_t origin = matrix.GetOrigin( );
		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) / 2.f;

		points.push_back( center );

		if ( index == HITBOX_R_FOOT || index == HITBOX_L_FOOT ) {
			float d1 = ( bbox->m_mins.z - center.z ) * 0.875f;

			if ( index == HITBOX_L_FOOT )
				d1 *= -1.f;

			points.push_back( { center.x, center.y, center.z + d1 } );

			if ( g_menu.main.aimbot.multipoint.get( 3 ) ) {
				float d2 = ( bbox->m_mins.x - center.x ) * scale;
				float d3 = ( bbox->m_maxs.x - center.x ) * scale;

				points.push_back( { center.x + d2, center.y, center.z } );

				points.push_back( { center.x + d3, center.y, center.z } );
			}
		}

		if ( points.empty( ) )
			return false;

		for ( auto& p : points ) {
			p = { p.dot( matrix[ 0 ] ), p.dot( matrix[ 1 ] ), p.dot( matrix[ 2 ] ) };

			p += origin;
		}
	}

	else {
		float r = bbox->m_radius * scale;
		float br = bbox->m_radius * bscale;

		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) / 2.f;
		points.push_back( center );

		if ( index == HITBOX_HEAD ) {
			if ( g_menu.main.aimbot.multipoint.get( 0 ) ) {
				constexpr float rotation = 0.70710678f;

				points.push_back( { bbox->m_maxs.x + ( rotation * r ), bbox->m_maxs.y + ( -rotation * r ), bbox->m_maxs.z } );

				points.push_back( { bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + r } );

				points.push_back( { bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - r } );

				points.push_back( { bbox->m_maxs.x, bbox->m_maxs.y - r, bbox->m_maxs.z } );

				CCSGOPlayerAnimState* state = record->m_player->m_PlayerAnimState( );

				if ( state && record->m_anim_velocity.length( ) <= 0.1f && record->m_eye_angles.x <= state->m_flAimPitchMin ) {
					points.push_back( { bbox->m_maxs.x - r, bbox->m_maxs.y, bbox->m_maxs.z } );
				}
			}
		}

		else if ( index == HITBOX_BODY ) {
			if ( g_menu.main.aimbot.multipoint.get( 2 ) )
				points.push_back( { center.x, bbox->m_maxs.y - br, center.z } );
		}

		else if ( index == HITBOX_PELVIS || index == HITBOX_UPPER_CHEST ) {
			points.push_back( { center.x, bbox->m_maxs.y - r, center.z } );
		}

		else if ( index == HITBOX_THORAX || index == HITBOX_CHEST ) {
			if ( g_menu.main.aimbot.multipoint.get( 1 ) )
				points.push_back( { center.x, bbox->m_maxs.y - r, center.z } );
		}

		else if ( index == HITBOX_R_CALF || index == HITBOX_L_CALF ) {
			if ( g_menu.main.aimbot.multipoint.get( 3 ) )
				points.push_back( { bbox->m_maxs.x - ( bbox->m_radius / 2.f ), bbox->m_maxs.y, bbox->m_maxs.z } );
		}

		else if ( index == HITBOX_R_THIGH || index == HITBOX_L_THIGH ) {
			points.push_back( center );
		}

		else if ( index == HITBOX_R_UPPER_ARM || index == HITBOX_L_UPPER_ARM ) {
			points.push_back( { bbox->m_maxs.x + bbox->m_radius, center.y, center.z } );
		}

		if ( points.empty( ) )
			return false;

		for ( auto& p : points )
			math::VectorTransform( p, bones[ bbox->m_bone ], p );
	}

	return true;
}


bool AimPlayer::GetBestAimPosition( vec3_t& aim, int& hitbox, float& damage, LagRecord* record ) {
	bool                  done, pen;
	float                 dmg, pendmg;
	HitscanData_t         scan;
	std::vector< vec3_t > points;

	int hp = std::min( 100, m_player->m_iHealth( ) );

	if ( g_cl.m_weapon_id == ZEUS ) {
		dmg = pendmg = hp;
		pen = true;
	}

	else {
		const auto min_dmg_override = ( g_menu.main.aimbot.override_min_dmg.get( ) && g_input.GetKeyState( g_menu.main.aimbot.override_min_dmg_key.get( ) ) );

		dmg = min_dmg_override ? g_menu.main.aimbot.override_dmg.get( ) 
			: g_menu.main.aimbot.minimal_damage.get( );

		if ( g_menu.main.aimbot.minimal_damage_hp.get( ) )
			dmg = std::ceil( ( dmg / 100.f ) * hp );

		pendmg = min_dmg_override ? g_menu.main.aimbot.override_dmg.get( ) 
			: g_menu.main.aimbot.penetrate_minimal_damage.get( );

		if ( g_menu.main.aimbot.penetrate_minimal_damage_hp.get( ) )
			pendmg = std::ceil( ( pendmg / 100.f ) * hp );

		pen = g_menu.main.aimbot.penetrate.get( );
	}

	const auto backup_origin = m_player->m_vecOrigin( );
	const auto backup_mins = m_player->m_vecMins( );
	const auto backup_maxs = m_player->m_vecMins( );
	const auto backup_bone_cache = m_player->m_pBoneCache( );

	record->cache( );

	for ( const auto& it : m_hitboxes ) {
		done = false;

		if ( !SetupHitboxPoints( record, record->m_bones.data( ), it.m_index, points ) )
			continue;

		for ( const auto& point : points ) {
			penetration::PenetrationInput_t in;

			in.m_damage = dmg;
			in.m_damage_pen = pendmg;
			in.m_can_pen = pen;
			in.m_target = m_player;
			in.m_from = g_cl.m_local;
			in.m_pos = point;

			if ( it.m_mode == HitscanMode::LETHAL || it.m_mode == HitscanMode::LETHAL2 )
				in.m_damage = in.m_damage_pen = 1.f;

			penetration::PenetrationOutput_t out;

			if ( penetration::run( &in, &out ) ) {
				if ( it.m_index == HITBOX_HEAD && out.m_hitgroup != HITGROUP_HEAD )
					continue;

				if ( it.m_mode == HitscanMode::PREFER )
					done = true;

				else if ( it.m_mode == HitscanMode::LETHAL && out.m_damage >= m_player->m_iHealth( ) )
					done = true;

				else if ( it.m_mode == HitscanMode::LETHAL2 && ( out.m_damage * 2.f ) >= m_player->m_iHealth( ) )
					done = true;

				else if ( it.m_mode == HitscanMode::NORMAL ) {
					if ( out.m_damage > scan.m_damage ) {
						scan.m_damage = out.m_damage;
						scan.m_hitbox = it.m_index;
						scan.m_pos = point;

						if ( point == points.front( ) && out.m_damage >= m_player->m_iHealth( ) )
							break;
					}
				}

				if ( done ) {
					scan.m_damage = out.m_damage;
					scan.m_pos = point;
					scan.m_hitbox = it.m_index;
					break;
				}
			}
		}

		if ( done )
			break;
	}

	m_player->m_vecOrigin( ) = backup_origin;
	m_player->m_vecMins( ) = backup_mins;
	m_player->m_vecMaxs( ) = backup_maxs;
	m_player->m_pBoneCache( ) = backup_bone_cache;

	if ( scan.m_damage > 0.f ) {
		aim = scan.m_pos;
		damage = scan.m_damage;
		hitbox = scan.m_hitbox;
		return true;
	}

	return false;
}

bool Aimbot::SelectTarget( LagRecord* record, const vec3_t& aim, float damage ) {
	float dist, fov, height;
	int   hp;

	if ( g_menu.main.aimbot.fov.get( ) ) {
		if ( math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, aim ) > g_menu.main.aimbot.fov_amount.get( ) )
			return false;
	}

	switch ( g_menu.main.aimbot.selection.get( ) ) {
	case 0:
		dist = ( record->m_pred_origin - g_cl.m_shoot_pos ).length( );

		if ( dist < m_best_dist ) {
			m_best_dist = dist;
			return true;
		}

		break;

	case 1:
		fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, aim );

		if ( fov < m_best_fov ) {
			m_best_fov = fov;
			return true;
		}

		break;

	case 2:
		if ( damage > m_best_damage ) {
			m_best_damage = damage;
			return true;
		}

		break;

	case 3:
		hp = std::min( 100, record->m_player->m_iHealth( ) );

		if ( hp < m_best_hp ) {
			m_best_hp = hp;
			return true;
		}

		break;

	case 4:
		if ( record->m_lag < m_best_lag ) {
			m_best_lag = record->m_lag;
			return true;
		}

		break;

	case 5:
		height = record->m_pred_origin.z - g_cl.m_local->m_vecOrigin( ).z;

		if ( height < m_best_height ) {
			m_best_height = height;
			return true;
		}

		break;

	default:
		return false;
	}

	return false;
}

void Aimbot::update( ) {
	bool attack, attack2;

	attack = ( g_cl.m_cmd->m_buttons & IN_ATTACK );
	attack2 = ( g_cl.m_weapon_id == REVOLVER && g_cl.m_cmd->m_buttons & IN_ATTACK2 );

	if ( attack || attack2 ) {
		if ( m_target ) {
			if ( m_record && !m_record->m_broke_lc )
				g_cl.m_cmd->m_tick = game::TIME_TO_TICKS( m_record->m_sim_time ) + game::TIME_TO_TICKS( g_cl.m_lerp );

			g_cl.m_cmd->m_view_angles = m_angle;

			if ( !g_menu.main.aimbot.silent.get( ) )
				g_csgo.m_engine->SetViewAngles( m_angle );
		}

		if ( g_menu.main.aimbot.nospread.get( ) && g_menu.main.config.mode.get( ) == 1 )
			NoSpread( );

		if ( g_menu.main.aimbot.norecoil.get( ) )
			g_cl.m_cmd->m_view_angles -= g_cl.m_local->m_aimPunchAngle( ) * g_csgo.weapon_recoil_scale->GetFloat( );

		g_shots.OnShotFire( m_target ? m_target : nullptr, m_target ? m_damage : -1.f, g_cl.m_weapon_info->m_bullets, m_target ? m_record : nullptr );

		g_cl.m_shot = true;
		*g_cl.m_packet = true;
	}
}

void Aimbot::NoSpread( ) {
	bool    attack2;
	vec3_t  spread, forward, right, up, dir;

	attack2 = ( g_cl.m_weapon_id == REVOLVER && ( g_cl.m_cmd->m_buttons & IN_ATTACK2 ) );

	spread = g_cl.m_weapon->CalculateSpread( g_cl.m_cmd->m_random_seed, attack2 );

	g_cl.m_cmd->m_view_angles -= { -math::rad_to_deg( std::atan( spread.length_2d( ) ) ), 0.f, math::rad_to_deg( std::atan2( spread.x, spread.y ) ) };
}