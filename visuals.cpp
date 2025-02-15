#include "includes.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld( ) {
	std::vector< IMaterial* > world, props;

	for ( uint16_t h{ g_csgo.m_material_system->FirstMaterial( ) }; h != g_csgo.m_material_system->InvalidMaterial( ); h = g_csgo.m_material_system->NextMaterial( h ) ) {
		IMaterial* mat = g_csgo.m_material_system->GetMaterial( h );
		if ( !mat )
			continue;

		if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "World textures" ) )
			world.push_back( mat );

		else if ( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "StaticProp textures" ) )
			props.push_back( mat );
	}

	if ( g_menu.main.visuals.world.get( ) == 1 ) {
		for ( const auto& w : world )
			w->ColorModulate( 0.17f, 0.16f, 0.18f );

		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		for ( const auto& p : props )
			p->ColorModulate( 0.5f, 0.5f, 0.5f );
	}

	else {
		for ( const auto& w : world )
			w->ColorModulate( 1.f, 1.f, 1.f );

		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != -1 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}

		for ( const auto& p : props )
			p->ColorModulate( 1.f, 1.f, 1.f );
	}

	if ( g_menu.main.visuals.transparent_props.get( ) ) {
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		for ( const auto& p : props )
			p->AlphaModulate( g_menu.main.visuals.transparent_props_opacity.get( ) / 100.f );
	}

	else {
		if ( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != -1 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}

		for ( const auto& p : props )
			p->AlphaModulate( 1.0f );
	}
}

void Visuals::ThirdpersonThink( ) {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	if ( !g_csgo.m_engine->IsInGame( ) )
		return;

	bool alive = g_cl.m_local && g_cl.m_local->alive( );

	static float transition_amt = 0.0f;

	if ( g_menu.main.visuals.thirdperson_transition.get( ) ) {
		if ( !m_thirdperson )
			transition_amt += std::clamp < float >( ( ( 255 / 0.8f ) * g_csgo.m_globals->m_frametime ) / 255.f, 0.5f, 1.f );
		else
			transition_amt -= std::clamp < float >( ( ( 255 / 0.8f ) * g_csgo.m_globals->m_frametime ) / 255.f, 0.5f, 1.f );
	}

	if ( g_menu.main.visuals.thirdperson_transition.get( ) ? ( m_thirdperson || transition_amt > 0.5f ) : m_thirdperson ) {
		if ( alive && !g_csgo.m_input->CAM_IsThirdPerson( ) )
			g_csgo.m_input->CAM_ToThirdPerson( );

		else if ( g_cl.m_local->m_iObserverMode( ) == 4 ) {
			if ( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
				g_csgo.m_input->CAM_ToFirstPerson( );
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode( ) = 5;
		}
	}

	else if ( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		g_csgo.m_input->CAM_ToFirstPerson( );
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	if ( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		g_csgo.m_engine->GetViewAngles( offset );

		math::AngleVectors( offset, &forward );

		offset.z = g_menu.main.visuals.thirdperson_transition.get( ) ? g_menu.main.visuals.thirdperson_distance.get( ) * ( std::sqrtf( 1.f - std::powf( transition_amt - 1.f, 2.f ) ) ) : g_menu.main.visuals.thirdperson_distance.get( );

		origin = g_cl.m_shoot_pos;

		filter.SetPassEntity( g_cl.m_local );

		g_csgo.m_engine_trace->TraceRay(
			Ray( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
			MASK_NPCWORLDSTATIC,
			( ITraceFilter* )&filter,
			&tr
		);

		math::clamp( tr.m_fraction, 0.f, 1.f );
		offset.z *= tr.m_fraction;

		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::ManualIndicators( ) {
	if ( !g_menu.main.antiaim.manual_aa.get( ) && !g_menu.main.antiaim.manual_aa_indicators.get( ) )
		return;

	if ( !g_cl.m_processing || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	const int arrow_size = 13;

	auto render_arrow = [ & ]( vec2_t position, float rotation, Color color ) {
		Vertex vertex_points[ 3 ];

		vertex_points[ 0 ] = { position.x + arrow_size, position.y + arrow_size };
		vertex_points[ 1 ] = { position.x - arrow_size, position.y };
		vertex_points[ 2 ] = { position.x + arrow_size, position.y - arrow_size };

		vertex_points[ 0 ] = render::RotateVertex( position, vertex_points[ 0 ], rotation );
		vertex_points[ 1 ] = render::RotateVertex( position, vertex_points[ 1 ], rotation );
		vertex_points[ 2 ] = render::RotateVertex( position, vertex_points[ 2 ], rotation );

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, vertex_points );
		};

	const auto center_x = g_cl.m_width / 2;
	const auto center_y = g_cl.m_height / 2;

	const auto back_arrow_pos = vec2_t{ center_x, center_y + arrow_size * 5 };
	const auto left_arrow_pos = vec2_t{ center_x - arrow_size * 5, center_y };
	const auto right_arrow_pos = vec2_t{ center_x + arrow_size * 5, center_y };

	const auto inactive_arrow_color = Color( 9, 9, 9, 95 );
	const auto dir_arrow_color = g_menu.main.antiaim.manual_aa_indicators_color.get( );

	render_arrow( back_arrow_pos, -90.0f, inactive_arrow_color );
	render_arrow( left_arrow_pos, 0.0f, inactive_arrow_color );
	render_arrow( right_arrow_pos, -180.0f, inactive_arrow_color );

	switch ( g_hvh.manual_dir ) {
	case manual_direction::left:
		render_arrow( left_arrow_pos, 0.0f, dir_arrow_color );
		break;
	case manual_direction::right:
		render_arrow( right_arrow_pos, -180.0f, dir_arrow_color );
		break;
	case manual_direction::back:
		render_arrow( back_arrow_pos, -90.0f, dir_arrow_color );
		break;
	}
}

void Visuals::Hitmarker( ) {
	if ( !g_menu.main.misc.hitmarker.get( ) )
		return;

	if ( g_csgo.m_globals->m_curtime > m_hit_end )
		return;

	if ( m_hit_duration <= 0.f )
		return;

	float complete = ( g_csgo.m_globals->m_curtime - m_hit_start ) / m_hit_duration;
	int x = g_cl.m_width / 2,
		y = g_cl.m_height / 2,
		alpha = ( 1.f - complete ) * 240;

	constexpr int line{ 6 };

	render::line( x - line, y - line, x - ( line / 4 ), y - ( line / 4 ), { 200, 200, 200, alpha } );
	render::line( x - line, y + line, x - ( line / 4 ), y + ( line / 4 ), { 200, 200, 200, alpha } );
	render::line( x + line, y + line, x + ( line / 4 ), y + ( line / 4 ), { 200, 200, 200, alpha } );
	render::line( x + line, y - line, x + ( line / 4 ), y - ( line / 4 ), { 200, 200, 200, alpha } );
}

void Visuals::NoSmoke( ) {
	if ( !smoke1 )
		smoke1 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_fire" ), XOR( "Other textures" ) );

	if ( !smoke2 )
		smoke2 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_smokegrenade" ), XOR( "Other textures" ) );

	if ( !smoke3 )
		smoke3 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods" ), XOR( "Other textures" ) );

	if ( !smoke4 )
		smoke4 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ), XOR( "Other textures" ) );

	if ( g_menu.main.visuals.nosmoke.get( ) ) {
		if ( !smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if ( !smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, true );
	}

	else {
		if ( smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if ( smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, false );
	}
}

void Visuals::think( ) {
	if ( !g_cl.m_local )
		return;

	if ( g_menu.main.visuals.noscope.get( )
		&& g_cl.m_local->alive( )
		&& g_cl.m_local->GetActiveWeapon( )
		&& g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped( ) ) {
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt( );

		if ( size > 1 ) {
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		render::rect_filled( 0, y, w, size, colors::black );
		render::rect_filled( x, 0, size, h, colors::black );
	}

	for ( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if ( !ent )
			continue;

		draw( ent );
	}

	SpreadCrosshair( );
	StatusIndicators( );
	Spectators( );
	PenetrationCrosshair( );
	Hitmarker( );
	DrawPlantedC4( );
	ManualIndicators( );
}

void Visuals::Spectators( ) {
	if ( !g_menu.main.visuals.spectators.get( ) )
		return;

	std::vector< std::string > spectators{ XOR( "spectators" ) };
	int h = render::menu_shade.m_size.m_height;

	for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if ( !player )
			continue;

		if ( player->m_bIsLocalPlayer( ) )
			continue;

		if ( player->dormant( ) )
			continue;

		if ( player->m_lifeState( ) == LIFE_ALIVE )
			continue;

		if ( player->GetObserverTarget( ) != g_cl.m_local )
			continue;

		player_info_t info;
		if ( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
			continue;

		spectators.push_back( std::string( info.m_name ).substr( 0, 24 ) );
	}

	size_t total_size = spectators.size( ) * ( h - 1 );

	for ( size_t i{ }; i < spectators.size( ); ++i ) {
		const std::string& name = spectators[ i ];

		render::menu_shade.string( g_cl.m_width - 20, ( g_cl.m_height / 2 ) - ( total_size / 2 ) + ( i * ( h - 1 ) ),
			{ 255, 255, 255, 179 }, name, render::ALIGN_RIGHT );
	}
}

void Visuals::StatusIndicators( ) {
	if ( !g_cl.m_processing )
		return;

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	if ( g_menu.main.visuals.indicators.get( 1 ) ) {
		if ( g_cl.m_local->m_vecVelocity( ).length_2d( ) > 270.f || g_cl.m_lagcomp ) {
			Indicator_t ind{ };
			ind.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
			ind.text = XOR( "LC" );

			indicators.push_back( ind );
		}
	}

	if ( g_menu.main.visuals.indicators.get( 0 ) ) {
		float change = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );

		Indicator_t ind{ };
		ind.color = change > 35.f ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LBY" );
		indicators.push_back( ind );
	}

	if ( g_menu.main.visuals.indicators.get( 2 ) ) {
		Indicator_t ind{ };
		ind.color = g_aimbot.m_fake_latency ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "PING" );

		indicators.push_back( ind );
	}

	const auto min_dmg_override = ( g_menu.main.aimbot.override_min_dmg.get( ) && g_input.GetKeyState( g_menu.main.aimbot.override_min_dmg_key.get( ) ) );

	if ( min_dmg_override ) {
		Indicator_t ind{ };
		ind.color = Color( 255, 255, 255 );
		ind.text = XOR( "DMG" );

		indicators.push_back( ind );
	}

	if ( indicators.empty( ) )
		return;

	for ( size_t i{ }; i < indicators.size( ); ++i ) {
		auto& indicator = indicators[ i ];

		render::indicator.string( 20, g_cl.m_height - 80 - ( 30 * i ), indicator.color, indicator.text );
	}
}

void Visuals::SpreadCrosshair( ) {
	if ( !g_cl.m_processing )
		return;

	if ( !g_menu.main.visuals.spread_xhair.get( ) )
		return;

	Weapon* weapon = g_cl.m_local->GetActiveWeapon( );
	if ( !weapon )
		return;

	WeaponInfo* data = weapon->GetWpnData( );
	if ( !data )
		return;

	CSWeaponType type = data->m_weapon_type;
	if ( type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE )
		return;

	float radius = ( ( weapon->GetInaccuracy( ) + weapon->GetSpread( ) ) * 320.f ) / ( std::tan( math::deg_to_rad( g_cl.m_local->GetFOV( ) ) * 0.5f ) + FLT_EPSILON );

	radius *= g_cl.m_height * ( 1.f / 480.f );

	Color col = g_menu.main.visuals.spread_xhair_col.get( );

	col.a( ) = 200 * ( g_menu.main.visuals.spread_xhair_blend.get( ) / 100.f );

	int segements = std::max( 16, ( int )std::round( radius * 0.75f ) );
	render::circle( g_cl.m_width / 2, g_cl.m_height / 2, radius, segements, col );
}

void Visuals::PenetrationCrosshair( ) {
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if ( !g_menu.main.visuals.pen_crosshair.get( ) || !g_cl.m_processing )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	valid_player_hit = ( g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy( g_cl.m_local ) );
	if ( valid_player_hit )
		final_color = colors::transparent_yellow;

	else if ( g_cl.m_pen_data.m_pen )
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	render::rect_filled( x - 1, y - 1, 3, 3, final_color );
}

void Visuals::draw( Entity* ent ) {
	if ( ent->IsPlayer( ) ) {
		Player* player = ent->as< Player* >( );

		if ( !player->alive( ) )
			return;

		if ( player->m_bIsLocalPlayer( ) )
			return;

		DrawPlayer( player );
	}

	else if ( g_menu.main.visuals.items.get( ) && ent->IsBaseCombatWeapon( ) && !ent->dormant( ) )
		DrawItem( ent->as< Weapon* >( ) );

	else if ( g_menu.main.visuals.proj.get( ) )
		DrawProjectile( ent->as< Weapon* >( ) );
}

void Visuals::DrawProjectile( Weapon* ent ) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin( );
	if ( !render::WorldToScreen( origin, screen ) )
		return;

	Color col = g_menu.main.visuals.proj_color.get( );
	col.a( ) = 0xb4;

	if ( ent->is( HASH( "CDecoyProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "DECOY" ), render::ALIGN_CENTER );

	else if ( ent->is( HASH( "CMolotovProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "MOLLY" ), render::ALIGN_CENTER );

	else if ( ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) ) {
		const model_t* model = ent->GetModel( );

		if ( model ) {
			std::string name{ ent->GetModel( )->m_name };

			if ( name.find( XOR( "flashbang" ) ) != std::string::npos )
				render::esp_small.string( screen.x, screen.y, col, XOR( "FLASH" ), render::ALIGN_CENTER );

			else if ( name.find( XOR( "fraggrenade" ) ) != std::string::npos ) {
				if ( g_menu.main.visuals.proj_range.get( 0 ) )
					render::sphere( origin, 350.f, 5.f, 1.f, g_menu.main.visuals.proj_range_color.get( ) );

				render::esp_small.string( screen.x, screen.y, col, XOR( "FRAG" ), render::ALIGN_CENTER );
			}
		}
	}

	else if ( ent->is( HASH( "CInferno" ) ) ) {
		if ( g_menu.main.visuals.proj_range.get( 1 ) )
			render::sphere( origin, 150.f, 5.f, 1.f, g_menu.main.visuals.proj_range_color.get( ) );

		render::esp_small.string( screen.x, screen.y, col, XOR( "FIRE" ), render::ALIGN_CENTER );
	}

	else if ( ent->is( HASH( "CSmokeGrenadeProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "SMOKE" ), render::ALIGN_CENTER );
}

void Visuals::DrawItem( Weapon* item ) {
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle( item->m_hOwnerEntity( ) );
	if ( owner )
		return;

	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin( );
	if ( !render::WorldToScreen( origin, screen ) )
		return;

	WeaponInfo* data = item->GetWpnData( );
	if ( !data )
		return;

	Color col = g_menu.main.visuals.item_color.get( );
	col.a( ) = 0xb4;

	if ( item->is( HASH( "CC4" ) ) )
		render::esp_small.string( screen.x, screen.y, { 150, 200, 60, 0xb4 }, XOR( "BOMB" ), render::ALIGN_CENTER );

	else {
		std::string name{ item->GetLocalizedName( ) };

		std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

		render::esp_small.string( screen.x, screen.y, col, name, render::ALIGN_CENTER );
	}

	if ( !g_menu.main.visuals.ammo.get( ) )
		return;

	if ( data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE )
		return;

	if ( item->m_iItemDefinitionIndex( ) == 0 || item->m_iItemDefinitionIndex( ) == C4 )
		return;

	std::string ammo = tfm::format( XOR( "(%i/%i)" ), item->m_iClip1( ), item->m_iPrimaryReserveAmmoCount( ) );
	render::esp_small.string( screen.x, screen.y - render::esp_small.m_size.m_height - 1, col, ammo, render::ALIGN_CENTER );
}

void Visuals::OffScreen( Player* player, int alpha ) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[ 3 ], verts_outline[ 3 ];
	Color  color;

	static auto get_offscreen_data = [ ]( const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation ) {
		ang_t  view_angles( g_csgo.m_view_render->m_view.m_angles );
		vec3_t fwd, right, up( 0.f, 0.f, 1.f );
		float  front, side, yaw_rad, sa, ca;

		math::AngleVectors( view_angles, &fwd );

		fwd.z = 0.f;
		fwd.normalize( );

		right = up.cross( fwd );
		front = delta.dot( fwd );
		side = delta.dot( right );

		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		out_rotation = math::rad_to_deg( std::atan2( out_offscreen_pos.x, out_offscreen_pos.y ) + math::pi );

		yaw_rad = math::deg_to_rad( -out_rotation );
		sa = std::sin( yaw_rad );
		ca = std::cos( yaw_rad );

		out_offscreen_pos.x = ( int )( ( g_cl.m_width / 2.f ) + ( radius * sa ) );
		out_offscreen_pos.y = ( int )( ( g_cl.m_height / 2.f ) - ( radius * ca ) );
		};

	if ( !g_menu.main.players.offscreen.get( ) )
		return;

	if ( !g_cl.m_processing || !g_cl.m_local->enemy( player ) )
		return;

	target_pos = player->WorldSpaceCenter( );
	is_on_screen = render::WorldToScreen( target_pos, screen_pos );

	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	if ( !is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >( g_cl.m_width + leeway_x )
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >( g_cl.m_height + leeway_y ) ) {
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		delta = ( target_pos - view_origin ).normalized( );

		radius = 200.f * ( g_cl.m_height / 480.f );

		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		offscreen_rotation = -offscreen_rotation;

		verts[ 0 ] = { offscreen_pos.x, offscreen_pos.y };
		verts[ 1 ] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f };
		verts[ 2 ] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f };

		verts_outline[ 0 ] = { verts[ 0 ].m_pos.x - 1.f, verts[ 0 ].m_pos.y - 1.f };
		verts_outline[ 1 ] = { verts[ 1 ].m_pos.x - 1.f, verts[ 1 ].m_pos.y + 1.f };
		verts_outline[ 2 ] = { verts[ 2 ].m_pos.x + 1.f, verts[ 2 ].m_pos.y + 1.f };

		verts[ 0 ] = render::RotateVertex( offscreen_pos, verts[ 0 ], offscreen_rotation );
		verts[ 1 ] = render::RotateVertex( offscreen_pos, verts[ 1 ], offscreen_rotation );
		verts[ 2 ] = render::RotateVertex( offscreen_pos, verts[ 2 ], offscreen_rotation );
		color = g_menu.main.players.offscreen_color.get( );
		color.a( ) = ( alpha == 255 ) ? alpha : alpha / 2;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, verts );
	}
}

void Visuals::DrawPlayer( Player* player ) {
	constexpr float MAX_DORMANT_TIME = 10.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;

	Rect		  box;
	player_info_t info;
	Color		  color;

	int index = player->index( );

	float& opacity = m_opacities[ index - 1 ];
	bool& draw = m_draw[ index - 1 ];

	constexpr int frequency = 1.f / 0.3f;

	float step = frequency * g_csgo.m_globals->m_frametime;

	bool enemy = player->enemy( g_cl.m_local );
	bool dormant = player->dormant( );

	if ( g_menu.main.visuals.enemy_radar.get( ) && enemy && !dormant )
		player->m_bSpotted( ) = true;

	if ( !dormant )
		draw = true;

	if ( !draw )
		return;

	dormant ? opacity -= step : opacity += step;

	bool dormant_esp = enemy && g_menu.main.players.dormant.get( );

	math::clamp( opacity, 0.f, 1.f );
	if ( !opacity && !dormant_esp )
		return;

	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime( );
	if ( dormant && dt > MAX_DORMANT_TIME )
		return;

	int alpha = ( int )( 255.f * opacity );
	int low_alpha = ( int )( 179.f * opacity );

	color = enemy ? g_menu.main.players.box_enemy.get( ) : g_menu.main.players.box_friendly.get( );

	if ( dormant && dormant_esp ) {
		alpha = 112;
		low_alpha = 80;

		if ( dt > DORMANT_FADE_TIME ) {
			float faded = ( dt - DORMANT_FADE_TIME );
			float scale = 1.f - ( faded / DORMANT_FADE_TIME );

			alpha *= scale;
			low_alpha *= scale;
		}

		color = { 112, 112, 112 };
	}

	color.a( ) = alpha;

	if ( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	OffScreen( player, alpha );

	if ( !GetPlayerBoxRect( player, box ) ) {
		return;
	}

	bool bone_esp = ( enemy && g_menu.main.players.skeleton.get( 0 ) ) || ( !enemy && g_menu.main.players.skeleton.get( 1 ) );
	if ( bone_esp )
		DrawSkeleton( player, alpha );

	bool box_esp = ( enemy && g_menu.main.players.box.get( 0 ) ) || ( !enemy && g_menu.main.players.box.get( 1 ) );

	if ( box_esp )
		render::rect_outlined( box.x, box.y, box.w, box.h, color, { 10, 10, 10, low_alpha } );

	bool name_esp = ( enemy && g_menu.main.players.name.get( 0 ) ) || ( !enemy && g_menu.main.players.name.get( 1 ) );

	if ( name_esp ) {
		std::string name{ std::string( info.m_name ).substr( 0, 24 ) };

		Color clr = g_menu.main.players.name_color.get( );
		clr.a( ) = low_alpha;

		render::esp.string( box.x + box.w / 2, box.y - render::esp.m_size.m_height, clr, name, render::ALIGN_CENTER );
	}

	bool health_esp = ( enemy && g_menu.main.players.health.get( 0 ) ) || ( !enemy && g_menu.main.players.health.get( 1 ) );

	if ( health_esp ) {
		int y = box.y + 1;
		int h = box.h - 2;

		int hp = std::min( 100, player->m_iHealth( ) );

		int r = std::min( ( 510 * ( 100 - hp ) ) / 100, 255 );
		int g = std::min( ( 510 * hp ) / 100, 255 );

		int fill = ( int )std::round( hp * h / 100.f );

		render::rect_filled( box.x - 6, y - 1, 4, h + 2, { 10, 10, 10, low_alpha } );

		render::rect( box.x - 5, y + h - fill, 2, fill, { r, g, 0, alpha } );

		if ( hp < 100 )
			render::esp_small.string( box.x - 5, y + ( h - fill ) - 5, { 255, 255, 255, low_alpha }, std::to_string( hp ), render::ALIGN_CENTER );
	}

	{
		std::vector< std::pair< std::string, Color > > flags;

		auto items = enemy ? g_menu.main.players.flags_enemy.GetActiveIndices( ) : g_menu.main.players.flags_friendly.GetActiveIndices( );

		for ( auto it = items.begin( ); it != items.end( ); ++it ) {
			if ( *it == 0 )
				flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 150, 200, 60, low_alpha } } );

			if ( *it == 1 ) {
				if ( player->m_bHasHelmet( ) && player->m_ArmorValue( ) > 0 )
					flags.push_back( { XOR( "HK" ), { 255, 255, 255, low_alpha } } );

				else if ( player->m_bHasHelmet( ) )
					flags.push_back( { XOR( "H" ), { 255, 255, 255, low_alpha } } );

				else if ( player->m_ArmorValue( ) > 0 )
					flags.push_back( { XOR( "K" ), { 255, 255, 255, low_alpha } } );
			}

			if ( *it == 2 && player->m_bIsScoped( ) )
				flags.push_back( { XOR( "ZOOM" ), { 60, 180, 225, low_alpha } } );

			if ( *it == 3 && player->m_flFlashBangTime( ) > 0.f )
				flags.push_back( { XOR( "FLASHED" ), { 255, 255, 0, low_alpha } } );

			if ( *it == 4 ) {
				C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

				if ( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 )
					flags.push_back( { XOR( "RELOAD" ), { 60, 180, 225, low_alpha } } );
			}

			if ( *it == 5 && player->HasC4( ) )
				flags.push_back( { XOR( "BOMB" ), { 255, 0, 0, low_alpha } } );
		}

		for ( size_t i{ }; i < flags.size( ); ++i ) {
			const auto& f = flags[ i ];

			int offset = i * ( render::esp_small.m_size.m_height - 1 );

			render::esp_small.string( box.x + box.w + 2, box.y + offset, f.second, f.first );
		}
	}

	{
		int  offset{ 0 };

		if ( enemy && g_menu.main.players.lby_update.get( ) ) {
			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];

			if ( data && data->m_moved && data->m_records.size( ) ) {
				LagRecord* current = data->m_records.front( ).get( );

				if ( current ) {
					if ( !( current->m_velocity.length_2d( ) > 0.1 && !current->m_fake_walk ) && data->m_body_index <= 3 ) {
						float cycle = std::clamp<float>( data->m_body_update - current->m_anim_time, 0.f, 1.0f );
						float width = ( box.w * cycle ) / 1.1f;

						if ( width > 0.f ) {
							render::rect_filled( box.x, box.y + box.h + 2, box.w, 4, { 10, 10, 10, low_alpha } );

							Color clr = g_menu.main.players.lby_update_color.get( );
							clr.a( ) = alpha;
							render::rect( box.x + 1, box.y + box.h + 3, width, 2, clr );

							offset += 5;
						}
					}
				}
			}
		}

		if ( ( enemy && g_menu.main.players.weapon.get( 0 ) ) || ( !enemy && g_menu.main.players.weapon.get( 1 ) ) ) {
			Weapon* weapon = player->GetActiveWeapon( );
			if ( weapon ) {
				WeaponInfo* data = weapon->GetWpnData( );
				if ( data ) {
					int bar;
					float scale;

					int max = data->m_max_clip1;
					int current = weapon->m_iClip1( );

					C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

					bool reload = ( layer1->m_weight != 0.f ) && ( player->GetSequenceActivity( layer1->m_sequence ) == 967 );

					if ( max != -1 && g_menu.main.players.ammo.get( ) ) {
						if ( reload )
							scale = layer1->m_cycle;

						else
							scale = ( float )current / max;

						bar = ( int )std::round( ( box.w - 2 ) * scale );

						render::rect_filled( box.x, box.y + box.h + 2 + offset, box.w, 4, { 10, 10, 10, low_alpha } );

						Color clr = g_menu.main.players.ammo_color.get( );
						clr.a( ) = alpha;
						render::rect( box.x + 1, box.y + box.h + 3 + offset, bar, 2, clr );

						if ( current <= ( int )std::round( max / 5 ) && !reload )
							render::esp_small.string( box.x + bar, box.y + box.h + offset, { 255, 255, 255, low_alpha }, std::to_string( current ), render::ALIGN_CENTER );

						offset += 6;
					}

					if ( g_menu.main.players.weapon_mode.get( ) == 0 ) {
						std::string name{ weapon->GetLocalizedName( ) };

						std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

						render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset, { 255, 255, 255, low_alpha }, name, render::ALIGN_CENTER );
					}

					else if ( g_menu.main.players.weapon_mode.get( ) == 1 ) {
						offset -= 5;

						std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[ weapon->m_iItemDefinitionIndex( ) ] );
						render::cs.string( box.x + box.w / 2, box.y + box.h + offset, { 255, 255, 255, low_alpha }, icon, render::ALIGN_CENTER );
					}
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4( ) {
	bool        mode_2d, mode_3d, is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = [ ]( float damage, int armor_value ) {
		float new_damage, armor;

		if ( armor_value > 0 ) {
			new_damage = damage * 0.5f;
			armor = ( damage - new_damage ) * 0.5f;

			if ( armor > ( float )armor_value ) {
				armor = ( float )armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max( 0, ( int )std::floor( damage ) );
		};

	mode_2d = g_menu.main.visuals.planted_c4.get( 0 );
	mode_3d = g_menu.main.visuals.planted_c4.get( 1 );
	if ( !mode_2d && !mode_3d )
		return;

	if ( !m_c4_planted )
		return;

	{
		dst = g_cl.m_local->WorldSpaceCenter( );
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length( );

		range_damage = m_planted_c4_damage * std::exp( ( dist * dist ) / ( ( m_planted_c4_radius_scaled * -2.f ) * m_planted_c4_radius_scaled ) );

		final_damage = scale_damage( range_damage, g_cl.m_local->m_ArmorValue( ) );
	}

	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	time_str = tfm::format( XOR( "%.2f" ), explode_time_diff );
	damage_str = tfm::format( XOR( "%i" ), final_damage );

	damage_color = ( final_damage < g_cl.m_local->m_iHealth( ) ) ? colors::white : colors::red;

	is_visible = render::WorldToScreen( m_planted_c4_explosion_origin, screen_pos );

	if ( mode_2d ) {
		if ( explode_time_diff > 0.f )
			render::esp.string( 2, 65, colors::white, time_str, render::ALIGN_LEFT );

		if ( g_cl.m_local->alive( ) )
			render::esp.string( 2, 65 + render::esp.m_size.m_height, damage_color, damage_str, render::ALIGN_LEFT );
	}

	if ( mode_3d && is_visible ) {
		if ( explode_time_diff > 0.f )
			render::esp_small.string( screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER );

		if ( g_cl.m_local->alive( ) )
			render::esp_small.string( screen_pos.x, ( int )screen_pos.y + render::esp_small.m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER );
	}
}

bool Visuals::GetPlayerBoxRect( Player* player, Rect& box ) {
	vec2_t flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	const auto min = player->m_vecMins( ) + player->GetAbsOrigin( );
	const auto max = player->m_vecMaxs( ) + player->GetAbsOrigin( );

	vec3_t points[ ] = {
			vec3_t( min.x, min.y, min.z ),
			vec3_t( min.x, min.y, max.z ),
			vec3_t( min.x, max.y, min.z ),
			vec3_t( min.x, max.y, max.z ),
			vec3_t( max.x, min.y, min.z ),
			vec3_t( max.x, min.y, max.z ),
			vec3_t( max.x, max.y, min.z ),
			vec3_t( max.x, max.y, max.z ),
	};

	if ( !render::WorldToScreen( points[ 3 ], flb ) || !render::WorldToScreen( points[ 5 ], brt ) || !render::WorldToScreen( points[ 0 ], blb ) || !render::WorldToScreen( points[ 4 ], frt ) || !render::WorldToScreen( points[ 2 ], frb ) || !render::WorldToScreen( points[ 1 ], brb ) || !render::WorldToScreen( points[ 6 ], blt ) || !render::WorldToScreen( points[ 7 ], flt ) ) {
		return false;
	}

	vec2_t arr[ ] = {
			flb,
			brt,
			blb,
			frt,
			frb,
			brb,
			blt,
			flt
	};

	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	for ( auto i = 1; i < 8; i++ ) {
		if ( left > arr[ i ].x )
			left = arr[ i ].x;

		if ( bottom < arr[ i ].y )
			bottom = arr[ i ].y;

		if ( right < arr[ i ].x )
			right = arr[ i ].x;

		if ( top > arr[ i ].y )
			top = arr[ i ].y;
	}

	box = {
			static_cast< int >( left ),
			static_cast< int >( top ),
			static_cast< int >( right - left ),
			static_cast< int >( bottom - top )
	};

	return true;
}

void Visuals::DrawHistorySkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	AimPlayer* data;
	LagRecord* record;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if ( !g_menu.main.misc.fake_latency.get( ) )
		return;

	model = player->GetModel( );
	if ( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	data = &g_aimbot.m_players[ player->index( ) - 1 ];
	if ( !data )
		return;

	record = g_resolver.FindLastRecord( data );
	if ( !record )
		return;

	for ( int i{ }; i < hdr->m_num_bones; ++i ) {
		bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		if ( !record->m_bones.data( ) )
			continue;

		record->m_bones.data( )->get_bone( bone_pos, i );
		record->m_bones.data( )->get_bone( parent_pos, parent );

		Color clr = player->enemy( g_cl.m_local ) ? g_menu.main.players.skeleton_enemy.get( ) : g_menu.main.players.skeleton_friendly.get( );
		clr.a( ) = opacity;

		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::DrawSkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[ 128 ];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	model = player->GetModel( );
	if ( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	if ( !player->SetupBones( matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime ) )
		return;

	for ( int i{ }; i < hdr->m_num_bones; ++i ) {
		bone = hdr->GetBone( i );
		if ( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		parent = bone->m_parent;
		if ( parent == -1 )
			continue;

		matrix->get_bone( bone_pos, i );
		matrix->get_bone( parent_pos, parent );

		Color clr = player->enemy( g_cl.m_local ) ? g_menu.main.players.skeleton_enemy.get( ) : g_menu.main.players.skeleton_friendly.get( );
		clr.a( ) = opacity;

		if ( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::RenderGlow( ) {
	Color   color;
	Player* player;

	if ( !g_cl.m_local )
		return;

	if ( !g_csgo.m_glow->m_object_definitions.Count( ) )
		return;

	float blend = g_menu.main.players.glow_blend.get( ) / 100.f;

	for ( int i{ }; i < g_csgo.m_glow->m_object_definitions.Count( ); ++i ) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[ i ];

		if ( !obj->m_entity || !obj->m_entity->IsPlayer( ) )
			continue;

		player = obj->m_entity->as< Player* >( );

		if ( player->m_bIsLocalPlayer( ) )
			continue;

		float& opacity = m_opacities[ player->index( ) - 1 ];

		bool enemy = player->enemy( g_cl.m_local );

		if ( enemy && !g_menu.main.players.glow.get( 0 ) )
			continue;

		if ( !enemy && !g_menu.main.players.glow.get( 1 ) )
			continue;

		if ( enemy )
			color = g_menu.main.players.glow_enemy.get( );

		else
			color = g_menu.main.players.glow_friendly.get( );

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = false;
		obj->m_color = { ( float )color.r( ) / 255.f, ( float )color.g( ) / 255.f, ( float )color.b( ) / 255.f };
		obj->m_alpha = opacity * blend;
	}
}

void Visuals::DrawHitboxMatrix( LagRecord* record, Color col, float time ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel( );
	if ( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if ( !hdr )
		return;

	set = hdr->GetHitboxSet( record->m_player->m_nHitboxSet( ) );
	if ( !set )
		return;

	for ( int i{ }; i < set->m_hitboxes; ++i ) {
		bbox = set->GetHitbox( i );
		if ( !bbox )
			continue;

		if ( bbox->m_radius <= 0.f ) {
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

			matrix3x4_t matrix;
			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], rot_matrix, matrix );

			ang_t bbox_angle;
			math::MatrixAngles( matrix, bbox_angle );

			vec3_t origin = matrix.GetOrigin( );

			g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r( ), col.g( ), col.b( ), 0, time );
		}

		else {
			matrix3x4_t matrix;
			g_csgo.AngleMatrix( bbox->m_angle, matrix );

			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], matrix, matrix );

			math::VectorTransform( bbox->m_mins, matrix, mins );
			math::VectorTransform( bbox->m_maxs, matrix, maxs );

			g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r( ), col.g( ), col.b( ), col.a( ), time, 0, 0 );
		}
	}
}

void Visuals::DrawBeams( ) {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if ( !g_cl.m_local )
		return;

	if ( !g_menu.main.visuals.impact_beams.get( ) )
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	if ( !g_cl.m_processing ) {
		if ( !vis_impacts->empty( ) )
			vis_impacts->clear( );
	}

	else {
		impact_count = vis_impacts->size( );
		if ( !impact_count )
			return;

		curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

		for ( size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ]( i );
			if ( !impact )
				continue;

			if ( std::abs( curtime - game::TICKS_TO_TIME( impact->m_tickbase ) ) > g_menu.main.visuals.impact_beams_time.get( ) ) {
				vis_impacts->erase( vis_impacts->begin( ) + i );

				continue;
			}

			if ( impact->m_ignore )
				continue;

			if ( i == ( impact_count - 1 ) )
				is_final_impact = true;

			else if ( ( i + 1 ) < impact_count && impact->m_tickbase != vis_impacts->operator[ ]( i + 1 ).m_tickbase )
				is_final_impact = true;

			else
				is_final_impact = false;

			if ( is_final_impact ) {
				start = impact->m_shoot_pos;

				dir = ( impact->m_impact_pos - start ).normalized( );
				dist = ( impact->m_impact_pos - start ).length( );

				end = start + ( dir * dist );

				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( XOR( "sprites/purplelaser1.vmt" ) );
				beam_info.m_pszModelName = XOR( "sprites/purplelaser1.vmt" );
				beam_info.m_flHaloScale = 0.f;
				beam_info.m_flLife = g_menu.main.visuals.impact_beams_time.get( );
				beam_info.m_flWidth = 2.f;
				beam_info.m_flEndWidth = 2.f;
				beam_info.m_flFadeLength = 0.f;
				beam_info.m_flAmplitude = 0.f;
				beam_info.m_flBrightness = 255.f;
				beam_info.m_flSpeed = 0.5f;
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;
				beam_info.m_bRenderable = true;
				beam_info.m_nFlags = 0;

				if ( !impact->m_hit_player ) {
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_color.get( ).b( );
				}

				else {
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_hurt_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_hurt_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_hurt_color.get( ).b( );
				}

				beam = game::CreateGenericBeam( beam_info );
				if ( beam ) {
					g_csgo.m_beams->DrawBeam( beam );

					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::DebugAimbotPoints( Player* player ) {
}