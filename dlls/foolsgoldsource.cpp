#include "foolsgoldsource.h"

namespace foolsgoldsource
{
	Engine gEngine;

	Engine::Engine()
	{
		// set up all the engine functions so they can be used
		this->engineFunctions.pfnPrecacheModel = pfnPrecacheModel;
		this->engineFunctions.pfnPrecacheSound = pfnPrecacheSound;
		this->engineFunctions.pfnAlertMessage = pfnAlertMessage;
		this->engineFunctions.pfnAllocString = pfnAllocString;
		this->engineFunctions.pfnPEntityOfEntOffset = pfnPEntityOfEntOffset;
		this->engineFunctions.pfnPEntityOfEntIndex = pfnPEntityOfEntIndex;
		this->engineFunctions.pfnGetGameDir = pfnGetGameDir;
		this->engineFunctions.pfnIsDedicatedServer = pfnIsDedicatedServer;
		this->engineFunctions.pfnPEntityOfEntIndexAllEntities = pfnPEntityOfEntIndexAllEntities;

		// install the engine functions and global variables
		g_engfuncs = this->engineFunctions;
		extern globalvars_t* gpGlobals;
		gpGlobals = &this->globalVariables;

		this->globalVariables.maxClients = 32;
		// TODO: possible to use a smart pointer under the hood for this?
		this->globalVariables.pStringBase = new char[2048];
		memset( (char *)this->globalVariables.pStringBase, 0, 2048 );
		this->iStringTableOffset = 1;

		// TODO: edict_t * 0 is worldspawn?
		for( int i = 0; i <= this->globalVariables.maxClients; i++ )
		{
			// TODO: player spawning should happen later - and call one of the server-side callbacks?
			shared_ptr<edict_t> edict = std::make_shared<edict_t>();
			edict->free = 0;
			edict->pvPrivateData = std::make_unique<char[]>(1).get(); // TODO: should be CBasePlayer's data
			edict->v.classname = ALLOC_STRING("player");
			edict->v.netname = 0;
			edict->v.flags = FL_CLIENT;
			this->edicts.push_back(edict);
		}

		this->strGameDir = "valve";
		this->bIsDedicatedServer = false;

		this->iMaxEdicts = 1024;
	}

	Engine::~Engine()
	{
		delete[] this->globalVariables.pStringBase;
	}

	const enginefuncs_t Engine::GetServerEngineFunctions()
	{
		return this->engineFunctions;
	}

	const globalvars_t Engine::GetServerGlobalVariables()
	{
		return this->globalVariables;
	}

	string Engine::GetGameDirectory()
	{
		return this->strGameDir;
	}

	void Engine::SetGameDirectory( string strGameDir )
	{
		this->strGameDir = strGameDir;
	}

	bool Engine::GetIsDedicatedServer()
	{
		return this->bIsDedicatedServer;
	}

	void Engine::SetIsDedicatedServer( bool bIsDedicatedServer )
	{
		this->bIsDedicatedServer = bIsDedicatedServer;
	}

	void Engine::SetMaxClients( int iMaxClients )
	{
		this->globalVariables.maxClients = iMaxClients;
	}

	/////////////////////////////////
	// Stubbed enginefuncs_t below //
	/////////////////////////////////

	int pfnPrecacheModel( char* s )
	{
		printf( "Precache %s\n", s );

		gEngine.models.push_back( string( s ) );

		return gEngine.models.size() - 1;
	}

	int pfnPrecacheSound( char* s )
	{
		printf( "Precache %s\n", s );

		gEngine.sounds.push_back( string( s ) );

		return gEngine.sounds.size() - 1;
	}

	void pfnAlertMessage( ALERT_TYPE atype, char *szFmt, ... )
	{
		printf( "%s", szFmt );
	}

	edict_t* pfnPEntityOfEntOffset( int iEntOffset )
	{
		if( iEntOffset >= gEngine.edicts.size() )
		{
			return nullptr;
		}
		else
		{
			return gEngine.edicts[iEntOffset].get();
		}
	}

	int pfnAllocString( const char* szValue )
	{
		globalvars_t globalVars = gEngine.GetServerGlobalVariables();
		// get the next unassigned part of the string table
		const char* pCurrentOffset = globalVars.pStringBase + gEngine.iStringTableOffset;
		// copy the new string to the next unassigned part of the string table
		strcpy( (char *)pCurrentOffset, szValue );

		// get the newly assigned string's location
		int iCurrentOffset = gEngine.iStringTableOffset;
		// update the location of the next unassigned part of the string table
		gEngine.iStringTableOffset += strlen( szValue );
		// return the newly assigned string's location
		return iCurrentOffset;
	}

	edict_t* pfnPEntityOfEntIndex( int iEntIndex )
	{
		edict_t* result;

		// TODO: is pfnPEntityOfEntOffset the same as EDICT_NUM?
		if( iEntIndex < 0 ||
			iEntIndex >= gEngine.iMaxEdicts ||
			( (result = pfnPEntityOfEntOffset( iEntIndex )) == nullptr || result->free || !result->pvPrivateData ) &&
			( iEntIndex >= gEngine.GetServerGlobalVariables().maxClients || result->free ) ) // this check on result fails if there are no edicts - never happens in GoldSource?
		{
			result = nullptr;
		}

		return result;
	}

	void pfnGetGameDir( char *szGetGameDir )
	{
		strcpy( szGetGameDir, gEngine.GetGameDirectory().c_str() );
	}

	int pfnIsDedicatedServer( void )
	{
		return gEngine.GetIsDedicatedServer();
	}

	edict_t* pfnPEntityOfEntIndexAllEntities( int iEntIndex )
	{
		edict_t* result;

		// TODO: is pfnPEntityOfEntOffset the same as EDICT_NUM?
		if( iEntIndex < 0 ||
			iEntIndex >= gEngine.iMaxEdicts ||
			( (result = pfnPEntityOfEntOffset( iEntIndex )) == nullptr || result->free || !result->pvPrivateData ) &&
			( iEntIndex > gEngine.GetServerGlobalVariables().maxClients || result->free ) ) // this check on result fails if there are no edicts - never happens in GoldSource?
		{
			result = nullptr;
		}

		return result;
	}
}