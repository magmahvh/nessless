#include "Damage.hpp"
#include "../Settings.hpp"
#include "../Render.hpp"
#include "../SDK/Math/Math.hpp"

void C_DamageMarker::Instance( )
{
	if ( !g_Settings->m_bDamageMarker )
	{
		if ( !m_aDmgMarkers.empty( ) )
			m_aDmgMarkers.clear( );

		return;
	}

	if ( m_aDmgMarkers.empty( ) )
		return;

	float_t flTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	for ( int i = 0; i < m_aDmgMarkers.size( ); i++ )
	{
		auto HitMarker = &m_aDmgMarkers[ i ];
		if ( flTime - HitMarker->m_flTime >= 1.0f )
		{
			m_aDmgMarkers.erase( m_aDmgMarkers.begin( ) + i );
			continue;
		}

		Vector vecScreenPosition = Vector( 0, 0, 0 );
		if ( g_Globals.m_Interfaces.m_DebugOverlay->ScreenPosition( HitMarker->m_vecOrigin, vecScreenPosition ) )
			continue;

		g_Render->RenderText( std::to_string( ( int )( HitMarker->m_flDamage ) ), ImVec2( vecScreenPosition.x, vecScreenPosition.y ), Color::White, true, true, g_Globals.m_Fonts.m_SegoeUI );
	}
}

void C_DamageMarker::OnRageBotFire( Vector vecOrigin, float_t flDamage )
{
	if ( !g_Settings->m_bDamageMarker )
	{
		if ( !m_aDmgMarkers.empty( ) )
			m_aDmgMarkers.clear( );

		return;
	}

	DamageMarker_t& DmgMarker = m_aDmgMarkers.emplace_back( );
	DmgMarker.m_flTime = g_Globals.m_Interfaces.m_GlobalVars->m_flRealTime;
	DmgMarker.m_vecOrigin = vecOrigin;
	DmgMarker.m_flDamage = flDamage;
	DmgMarker.m_flStartZ = vecOrigin.z;
}