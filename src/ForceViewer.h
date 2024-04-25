/**********************************************************************
 *<
	FILE: ForceViewer.h

	DESCRIPTION: View the effects of space warps

	CREATED BY: John Burnett

	HISTORY: Created 10.17.2001

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#ifndef __FORCEVIEWER__H
#define __FORCEVIEWER__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

extern TCHAR *GetString(int id);
extern HINSTANCE hInstance;

#define FORCEVIEWER_CLASSID	Class_ID(0xf016797, 0x762b0972)

// Blur plugins that are causing crashes
#define BLURWIND_CLASSID	Class_ID(0x1c915e12, 0x5c8e760e)
#define RANDOMWALK_CLASSID	Class_ID(0x70ab0669, 0x734f496d)

#define NUM_REFS	1
#define REF_PBLOCK	0

enum {
	forceviewer_params
};

enum {
	fv_sizex,
	fv_sizey,
	fv_sizez,
	fv_resx,
	fv_resy,
	fv_resz,
	fv_forcenodes,
	fv_vectorscale,
	fv_displayvectorbases,
	fv_displaylattice,
};

#define RES_MIN		1
#define RES_MAX		999
#define RES_INIT	51
#define DISPLAY_SIZE_MIN	0.f
#define DISPLAY_SIZE_MAX	999999.f

class ForceViewer : public HelperObject {
	public:
		IParamBlock2* pblock;
		IObjParam *ip;

		HWND hAboutRoll;

		Tab<Point3> m_sampleForces;
		Tab<Point3> m_samplePoints;

		Point3 size;
		IPoint3 res;
		float vectorScale;

		BOOL isValid;
		TimeValue lastTime;

		ForceViewer();
		~ForceViewer();

		// Animatable
		Class_ID ClassID() { return FORCEVIEWER_CLASSID; }
		TCHAR *GetObjectName() const { return GetString(IDS_FORCEVIEWER_CLASSNAME); }
		
#if MAX_RELEASE_R24
		void GetClassName(MSTR& s, bool localized) const 
		{ 
			s = localized ? GetString(IDS_FORCEVIEWER_CLASSNAME) : _T("ForceViewer");
		};

#else
		void GetClassName(TSTR& s) { s = GetObjectName(); }
#endif
		
		
		void DeleteThis() { delete this; }

		int NumSubs() { return 1; }
		Animatable* SubAnim(int i) { return pblock; }

#if MAX_RELEASE_R24
		TSTR SubAnimName(int i, BOOL isLocalized) { return TSTR(GetString(IDS_PARAMETERS)); };
#else
		TSTR SubAnimName(int i) { return TSTR(GetString(IDS_PARAMETERS)); };
#endif

		int	NumParamBlocks() { return 1; }
		IParamBlock2* GetParamBlock(int i) { return pblock; }
		IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; }

		void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
		void EndEditParams(IObjParam *ip, ULONG flags,Animatable *next);

		// Reference Maker
		int NumRefs() { return NUM_REFS; }
		RefTargetHandle GetReference(int i) { return pblock; }
		void SetReference(int i, RefTargetHandle rtarg) { pblock = (IParamBlock2*)rtarg; }

#if MAX_VERSION_MAJOR < 17 //Max 2015
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
#else
		RefResult NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message, BOOL propagate);
#endif


		// Reference Target
		RefTargetHandle Clone(RemapDir& remap);

		// Base Object
		CreateMouseCallBack* GetCreateMouseCallBack();
		int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt);
		int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags);

		// Object
		int DoOwnSelectHilite() { return TRUE; }
		ObjectState Eval(TimeValue time) { return ObjectState(this); }
		void InitNodeName(TSTR& s) { s = GetObjectName(); }
		int UsesWireColor() { return TRUE; }
		Interval ObjectValidity(TimeValue t);
		void GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3 &box);
		void GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3 &box);

		BOOL CanCacheObject() { return FALSE; }
		void WSStateInvalidate() { isValid = FALSE; }
		BOOL IsWorldSpaceObject() { return TRUE; }

		// ForceViewer
		void Update(TimeValue t, INode* inode);
		void GetVectorCubeBound(Box3 &box);

		void DrawCube(GraphicsWindow* gw);
		void DrawVectors(GraphicsWindow* gw);
		void DrawVectorBases(ViewExp* vpt, GraphicsWindow* gw, Matrix3 tm);
		void DrawLattice(GraphicsWindow* gw);
};

#endif // __FORCEVIEWER__H
