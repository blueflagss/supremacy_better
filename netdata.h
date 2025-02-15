#pragma once

#define EQUAL_EPSILON 0.001

#define ADD_NETVAR( container, table, prop, tolerance ) \
    container.push_back( { g_netvars.get( HASH( table ), HASH( prop ) ), { }, tolerance } )

#define ADD_DATAMAP_VAR( container, datamap, prop, tolerance ) \
    container.push_back( { g_netvars.GetOffset( datamap, HASH( prop ) ), { }, tolerance } )

extern float AssignRangeMultiplier( int bits, double range );

class NetData {
private:
    inline float GetNew( float old_val, float new_val, float tolerance ) {
        const float delta = new_val - old_val;
        return fabsf( delta ) <= tolerance ? old_val : new_val;
    }

    inline vec3_t GetNew( vec3_t old_val, vec3_t new_val, float tolerance ) {
        return { GetNew( old_val.x, new_val.x, tolerance ),
                 GetNew( old_val.y, new_val.y, tolerance ),
                 GetNew( old_val.z, new_val.z, tolerance )
        };
    }

    template< typename T >
    struct CompressedNetVar {
        size_t m_offset{ };
        T m_last_value{ };
        float m_tolerance{ };
    };

public:
	void init( Player* player );
	void pre_update( Player* player );
	void post_update( Player* player );

    bool written_pre_vars = false;
    bool initialized = false;

    std::vector< CompressedNetVar< vec3_t > > vec3d_vars;
    std::vector< CompressedNetVar< float > > float_vars;
};

extern NetData g_netdata;