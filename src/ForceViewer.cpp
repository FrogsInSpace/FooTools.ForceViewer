/**********************************************************************
 *<
	FILE: ForceViewer.cpp

	DESCRIPTION: View the effects of space warps

	CREATED BY: John Burnett

	HISTORY: Created 10.17.2001

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "ForceViewer.h"
#include "AboutRollup.h"

#if MAX_RELEASE_R24
static Matrix3 IdentityMatrix();
#else
static Matrix3 IdentityMatrix(TRUE);
#endif

class ForceViewerCreateCallBack : public CreateMouseCallBack
{
public:
	ForceViewerCreateCallBack();

	int		proc	(ViewExp* vpt, int msg, int pointCount, int flags, IPoint2 Ps, Matrix3& mat);
	void	SetObj	(ForceViewer* obj) { m_obj = obj; }

private:
	ForceViewer*	m_obj;
	IPoint2			m_initPs;
	Point3			m_initPw;
	static BOOL		m_showLattice;
};

BOOL ForceViewerCreateCallBack::m_showLattice = TRUE;

ForceViewerCreateCallBack::ForceViewerCreateCallBack() :
	m_obj(NULL)
{
}

int ForceViewerCreateCallBack::proc(ViewExp* vpt, int msg, int pointCount, int flags, IPoint2 Ps, Matrix3& mat)
{
	switch (msg)
	{
		case MOUSE_FREEMOVE:
		{
			vpt->SnapPreview(Ps, Ps, NULL, SNAP_IN_3D);
			break;
		}

		case MOUSE_POINT:
		case MOUSE_MOVE:
		{
			if (!m_obj) return CREATE_ABORT;

			IParamBlock2* pblock = m_obj->GetParamBlock(forceviewer_params);
			if (!pblock) return CREATE_ABORT;

			switch (pointCount)
			{
				case 0:
				{
					m_initPs = Ps;
					m_initPw = vpt->SnapPoint(Ps, Ps, NULL, SNAP_IN_3D);
					mat.SetTrans(m_initPw);

					m_showLattice = pblock->GetInt(fv_displaylattice);
					pblock->SetValue(fv_displaylattice, 0, TRUE);

					break;
				}
				case 1:
				{
					mat.SetTrans(m_initPw);

					Point3 Pw = vpt->SnapPoint(Ps, Ps, NULL, SNAP_IN_3D);
					float radius = Length(Pw-m_initPw) * 2.0f;

					pblock->SetValue(fv_sizex, 0, radius);
					pblock->SetValue(fv_sizey, 0, radius);
					pblock->SetValue(fv_sizez, 0, (radius*(1.0f/(float)RES_INIT)));

					if (msg==MOUSE_POINT)
					{
						pblock->SetValue(fv_displaylattice, 0, m_showLattice);
						return (Length(Ps-m_initPs)<3)?CREATE_ABORT:CREATE_STOP;
					}
					break;
				}
			}
			break;
		}

		case MOUSE_UNINIT:
		{
			m_obj = NULL;
			break;
		}

		case MOUSE_ABORT:
		{
			if (!m_obj) return CREATE_ABORT;

			IParamBlock2* pblock = m_obj->GetParamBlock(forceviewer_params);
			if (!pblock) return CREATE_ABORT;

			pblock->SetValue(fv_displaylattice, 0, m_showLattice);

			return CREATE_ABORT;
		}
	}

	return CREATE_CONTINUE;
}

static ForceViewerCreateCallBack forceViewerCreateCallBack;

///////////////////////////////////////////////////////////////////////////

class ForceViewerClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new ForceViewer(); }
	const TCHAR *	ClassName() { return GetString(IDS_FORCEVIEWER_CLASSNAME); }

#if MAX_VERSION_MAJOR >= 24
	const TCHAR* NonLocalizedClassName() { return ClassName(); }
#endif

	SClass_ID		SuperClassID() { return HELPER_CLASS_ID; }
	Class_ID		ClassID() { return FORCEVIEWER_CLASSID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("ForceViewer"); }
	HINSTANCE		HInstance() { return hInstance; }
};

static ForceViewerClassDesc forceViewerClassDesc;
ClassDesc2* GetForceViewerDesc() { return &forceViewerClassDesc; }

///////////////////////////////////////////////////////////////////////////

class ForceViewerAccessor : public PBAccessor
{
	public:
		void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t)
		{
			ForceViewer* obj = (ForceViewer*)owner;

			switch(id)
			{
				case(fv_sizex):
				case(fv_sizey):
				case(fv_sizez):
				{
					if (v.f < DISPLAY_SIZE_MIN) v.f = DISPLAY_SIZE_MIN;
					else if (v.f > DISPLAY_SIZE_MAX) v.f = DISPLAY_SIZE_MAX;
					break;
				}
				case(fv_resx):
				case(fv_resy):
				case(fv_resz):
				{
					if (v.i < RES_MIN) v.i = RES_MIN;
					else if (v.i > RES_MAX) v.i = RES_MAX;
					break;
				}
			}
		}
};

static ForceViewerAccessor forceViewerAccessor;

///////////////////////////////////////////////////////////////////////////

class ForceViewerValidator : public PBValidator
{
	public:
		BOOL Validate(PB2Value& v);
		BOOL IsValidNode(INode* inode);
};

BOOL ForceViewerValidator::Validate(PB2Value& v)
{
	if (v.r)
		return IsValidNode((INode*)v.r);
	else
		return FALSE;
}

BOOL ForceViewerValidator::IsValidNode(INode* inode)
{
	Object* obj = inode->GetObjectRef();
	obj = obj->FindBaseObject();
	Class_ID id = obj->ClassID();

	return (
		obj &&
		obj->SuperClassID() == WSM_OBJECT_CLASS_ID &&
		id != BLURWIND_CLASSID &&
		id != RANDOMWALK_CLASSID
	);
}

static ForceViewerValidator forceViewerValidator;

///////////////////////////////////////////////////////////////////////////

#if MAX_VERSION_MAJOR >= 15
#define end p_end
#endif

static ParamBlockDesc2 forceviewer_pblock
(
	forceviewer_params, _T("Parameters"),  0, &forceViewerClassDesc,
	P_AUTO_CONSTRUCT + P_AUTO_UI, REF_PBLOCK,
	//rollout
	IDD_FORCEVIEWER, IDS_PARAMETERS, 0, 0, NULL,
	// params
	fv_sizex, _T("sizeX"), TYPE_FLOAT, P_RESET_DEFAULT, IDS_SIZEX,
		p_accessor,		&forceViewerAccessor,
		p_default, 		1.0f,
		p_range, 		DISPLAY_SIZE_MIN, DISPLAY_SIZE_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZEX_EDIT, IDC_SIZEX_SPIN, SPIN_AUTOSCALE,
		end,
	fv_sizey, _T("sizeY"), TYPE_FLOAT, P_RESET_DEFAULT, IDS_SIZEY,
		p_accessor,		&forceViewerAccessor,
		p_default, 		1.0f,
		p_range, 		DISPLAY_SIZE_MIN, DISPLAY_SIZE_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZEY_EDIT, IDC_SIZEY_SPIN, SPIN_AUTOSCALE,
		end,
	fv_sizez, _T("sizeZ"), TYPE_FLOAT, P_RESET_DEFAULT, IDS_SIZEZ,
		p_accessor,		&forceViewerAccessor,
		p_default, 		1.0f,
		p_range, 		DISPLAY_SIZE_MIN, DISPLAY_SIZE_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_SIZEZ_EDIT, IDC_SIZEZ_SPIN, SPIN_AUTOSCALE,
		end,
	fv_resx, _T("resolutionX"), TYPE_INT, 0, IDS_RESX,
		p_accessor,		&forceViewerAccessor,
		p_default, 		RES_INIT,
		p_range, 		RES_MIN, RES_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_RESX_EDIT, IDC_RESX_SPIN, 1.0f,
		end,
	fv_resy, _T("resolutionY"), TYPE_INT, 0, IDS_RESY,
		p_accessor,		&forceViewerAccessor,
		p_default, 		RES_INIT,
		p_range, 		RES_MIN, RES_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_RESY_EDIT, IDC_RESY_SPIN, 1.0f,
		end,
	fv_resz, _T("resolutionZ"), TYPE_INT, 0, IDS_RESZ,
		p_accessor,		&forceViewerAccessor,
		p_default, 		1,
		p_range, 		RES_MIN, RES_MAX,
		p_ui, 			TYPE_SPINNER, EDITTYPE_INT, IDC_RESZ_EDIT, IDC_RESZ_SPIN, 1.0f,
		end,
	fv_forcenodes, _T("forceNodes"), TYPE_INODE_TAB, 0, P_VARIABLE_SIZE, IDS_FORCENODES,
		p_ui,			TYPE_NODELISTBOX, IDC_FORCENODES, IDC_FORCENODE_ADD, 0, IDC_FORCENODE_REMOVE,
		p_validator,	&forceViewerValidator,
		end,
	fv_vectorscale, _T("vectorScale"), TYPE_FLOAT, 0, IDS_VECTORSCALE,
		p_default, 		10.0f,
		p_range, 		-999999.f, 999999.f,
		p_ui, 			TYPE_SPINNER, EDITTYPE_FLOAT, IDC_VECTORSCALE_EDIT, IDC_VECTORSCALE_SPIN, SPIN_AUTOSCALE,
		end,
	fv_displayvectorbases, _T("displayVectorBases"), TYPE_BOOL, 0, IDS_DISPLAY_VECTORBASES,
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_DISPLAY_VECTORBASES,
		end,
	fv_displaylattice, _T("displayLattice"), TYPE_BOOL, 0, IDS_DISPLAY_LATTICE,
		p_default, 		FALSE,
		p_ui, 			TYPE_SINGLECHEKBOX, IDC_DISPLAY_LATTICE,
		end,
	end
);

///////////////////////////////////////////////////////////////////////////


ForceViewer::ForceViewer()
{
	SetAFlag(A_OBJ_CREATING);

	ip = NULL;

	hAboutRoll = 0;

	pblock = NULL;
	forceViewerClassDesc.MakeAutoParamBlocks(this);
	assert(pblock);

	isValid = FALSE;
}

ForceViewer::~ForceViewer()
{
	DeleteAllRefsFromMe();
}

void ForceViewer::BeginEditParams(IObjParam* ip, ULONG flags, Animatable* prev)
{
	this->ip = ip;

	forceViewerClassDesc.BeginEditParams(ip, this, flags, prev);

	hAboutRoll = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_ABOUT), aboutDlgProc, _T("About"));
}

void ForceViewer::EndEditParams(IObjParam* ip, ULONG flags, Animatable* next)
{
	ClearAFlag(A_OBJ_CREATING);

	forceViewerClassDesc.EndEditParams(ip, this, flags, next);
	ip->DeleteRollupPage(hAboutRoll);

	this->ip = NULL;
}


#if MAX_VERSION_MAJOR < 17 //Max 2015
RefResult ForceViewer::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
	PartID& partID, RefMessage message)
#else
RefResult ForceViewer::NotifyRefChanged(const Interval& changeInt, RefTargetHandle hTarget,
	PartID& partID, RefMessage message, BOOL propagate)
#endif
{
	switch (message)
	{
		case REFMSG_CHANGE:
		{
			isValid = FALSE;
			break;
		}
// how to handle undo (ie. delete warp, then undo... doesn't get re-added to list doing this)
/*
		case REFMSG_REF_DELETED:
		{
			int i;
			if (pblock->LastNotifyParamID(i) == fv_forcenodes)
				pblock->Delete(fv_forcenodes, i, 1);
			break;
		}
*/
	}
	return(REF_SUCCEED);
}

RefTargetHandle ForceViewer::Clone(RemapDir& remap)
{
	ForceViewer* newObj = new ForceViewer();
	newObj->ReplaceReference(REF_PBLOCK, pblock->Clone(remap));
	newObj->m_sampleForces.SetCount(0);
	newObj->m_samplePoints.SetCount(0);
	newObj->isValid = FALSE;

#if MAX_RELEASE > 3100
	BaseClone(this, newObj, remap);
#endif

	return(newObj);
}

CreateMouseCallBack* ForceViewer::GetCreateMouseCallBack()
{
	forceViewerCreateCallBack.SetObj(this);
	return &forceViewerCreateCallBack;
}

#define CHECKHIT(drawFunc)			\
	{								\
		gw->clearHitCode();			\
		drawFunc;					\
		if (gw->checkHitCode()) {	\
			gw->setRndLimits(rlim);	\
			return TRUE;			\
		}							\
	}

int ForceViewer::HitTest(TimeValue t, INode *inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
{
	Update(t, inode);

	GraphicsWindow *gw = vpt->getGW();
	Matrix3 objectToWorld = inode->GetObjectTM(t);
	gw->setTransform(objectToWorld);

	DWORD rlim  = gw->getRndLimits();
	gw->setRndLimits((rlim|GW_PICK|GW_WIREFRAME) & ~(GW_ILLUM|GW_BACKCULL|GW_FLAT|GW_SPECULAR));

	HitRegion hitRegion;
	MakeHitRegion(hitRegion, type, crossing, 4, p);
	gw->setHitRegion(&hitRegion);

	CHECKHIT(DrawCube(gw))

	if (!(rlim & GW_BOX_MODE))
	{
		CHECKHIT(DrawVectors(gw))
		if (pblock->GetInt(fv_displayvectorbases, t))
			CHECKHIT(DrawVectorBases(vpt, gw, objectToWorld))
		if (pblock->GetInt(fv_displaylattice, t))
			CHECKHIT(DrawLattice(gw))
	}

	return FALSE;
}

int ForceViewer::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	Update(t, inode);

	GraphicsWindow *gw = vpt->getGW();
	Matrix3 objectToWorld = inode->GetObjectTM(t);
	gw->setTransform(objectToWorld);

	DWORD rlim  = gw->getRndLimits();
	gw->setRndLimits(GW_WIREFRAME|(rlim&GW_Z_BUFFER?GW_Z_BUFFER:0));

	if (inode->Selected())
		gw->setColor(LINE_COLOR, GetUIColor(COLOR_SELECTION));
	else if(inode->IsFrozen())
		gw->setColor(LINE_COLOR, GetUIColor(COLOR_FREEZE));
	else {
		DWORD col = inode->GetWireColor();
		gw->setColor(LINE_COLOR, Point3(GetRValue(col)/255.f, GetGValue(col)/255.f, GetBValue(col)/255.f));
	}

	DrawCube(gw);

	if(!(rlim & GW_BOX_MODE))
	{
		DrawVectors(gw);
		if (pblock->GetInt(fv_displayvectorbases, t))
			DrawVectorBases(vpt, gw, objectToWorld);
		if (pblock->GetInt(fv_displaylattice, t))
			DrawLattice(gw);
	}

	gw->setRndLimits(rlim);
	return(0);
}

Interval ForceViewer::ObjectValidity(TimeValue t)
{
	// This SHOULD work, but some spacewarps don't return a
	// proper validity (ie. wind with turbulence)
/*
	Interval valid = FOREVER;

	pblock->GetValidity(t, valid);

	for (int i=0; i<pblock->Count(fv_forcenodes); i++)
	{
		INode* inode = pblock->GetINode(fv_forcenodes, t, i);
		if (inode)
		{
			inode->GetNodeTM(t, &valid);
			Object* obj = inode->GetObjOrWSMRef();
			if (obj) valid &= obj->ObjectValidity(t);
		}
	}

	return valid;
*/
	return Interval(t, t);
}

void ForceViewer::GetWorldBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
{
	Update(t, inode);

	GetVectorCubeBound(box);

	box = box * inode->GetObjectTM(t);
}

void ForceViewer::GetLocalBoundBox(TimeValue t, INode* inode, ViewExp* vpt, Box3& box)
{
	Update(t, inode);

	GetVectorCubeBound(box);
}

void ForceViewer::Update(TimeValue t, INode* inode)
{
	size.x = pblock->GetFloat(fv_sizex, t);
	size.y = pblock->GetFloat(fv_sizey, t);
	size.z = pblock->GetFloat(fv_sizez, t);

	res.x = pblock->GetInt(fv_resx, t);
	res.y = pblock->GetInt(fv_resy, t);
	res.z = pblock->GetInt(fv_resz, t);

	vectorScale = pblock->GetFloat(fv_vectorscale, t);

	int cubeCount = res.x*res.y*res.z;
	if (cubeCount != m_sampleForces.Count())
	{
		m_sampleForces.SetCount(cubeCount);
		m_samplePoints.SetCount(cubeCount);
	}

	if (isValid && lastTime == t) return;

	for (int ii=0; ii<m_sampleForces.Count(); ++ii)
	{
		m_sampleForces[ii] = Point3::Origin;
		m_samplePoints[ii] = Point3::Origin;
	}

	// Run through and sample each spacewarp, storing forces as world space vectors

	Matrix3 objectToWorld = inode->GetObjectTM(t);

	for (int nIdx=0; nIdx<pblock->Count(fv_forcenodes); nIdx++)
	{
		INode* forceINode = pblock->GetINode(fv_forcenodes, t, nIdx);

		if (forceINode && forceViewerValidator.IsValidNode(forceINode))
		{
			WSMObject* wsObj = (WSMObject*)forceINode->GetObjectRef();

			ForceField* ff = wsObj->GetForceField(forceINode);

			if (ff)
			{
				static Point3 fakeVel(Point3::ZAxis);

				Point3 delta(
					(res.x>1) ? size.x/float(res.x-1) : 0.0f,
					(res.y>1) ? size.y/float(res.y-1) : 0.0f,
					(res.z>1) ? size.z/float(res.z-1) : 0.0f
				);

				Point3 minPoint(
					(res.x>1) ? -size.x*0.5f : 0.0f,
					(res.y>1) ? -size.y*0.5f : 0.0f,
					(res.z>1) ? -size.z*0.5f : 0.0f
				);

				Point3 samplePoint = minPoint;

				int vIdx = 0;
				for (int iz=0; iz<res.z; ++iz)
				{
					for (int iy=0; iy<res.y; ++iy)
					{
						for (int ix=0; ix<res.x; ++ix, ++vIdx)
						{
							m_samplePoints[vIdx] = samplePoint;
							m_sampleForces[vIdx] += 160000.0f * ff->Force(t, objectToWorld.PointTransform(samplePoint), fakeVel, vIdx);

							samplePoint.x += delta.x;
						}
						samplePoint.x = minPoint.x;
						samplePoint.y += delta.y;
					}
					samplePoint.y = minPoint.y;
					samplePoint.z += delta.z;
				}

				ff->DeleteThis();
			}
		}
	}

	// Scale forces and transform them into object space for later display

	for (int ii=0; ii<m_sampleForces.Count(); ++ii)
	{
		m_sampleForces[ii] *= vectorScale;

		Matrix3 worldToObject = Inverse(objectToWorld);
		m_sampleForces[ii] = worldToObject.VectorTransform(m_sampleForces[ii]);
	}

	isValid = TRUE;
	lastTime = t;
}

void ForceViewer::GetVectorCubeBound(Box3 &box)
{
	Point3 min; min.x = min.y = min.z = 0.f;
	Point3 max; max.x = max.y = max.z = 0.f;
	Point3 v;
	for (int i=0; i<m_sampleForces.Count(); i++)
	{
		v = m_sampleForces[i];
		if (v.x < min.x) min.x = v.x; else if (v.x > max.x) max.x = v.x;
		if (v.y < min.y) min.y = v.y; else if (v.y > max.y) max.y = v.y;
		if (v.z < min.z) min.z = v.z; else if (v.z > max.z) max.z = v.z;
	}

	box.pmax.x = size.x * 0.5f;
	box.pmax.y = size.y * 0.5f;
	box.pmax.z = size.z * 0.5f;

	box.pmin.x = -box.pmax.x;
	box.pmin.y = -box.pmax.y;
	box.pmin.z = -box.pmax.z;

	box.pmin = box.pmin + min;
	box.pmax = box.pmax + max;
}

void ForceViewer::DrawCube(GraphicsWindow* gw)
{
	Box3 box;
	Point3 pt[5];

	box.pmax.x = size.x * 0.5f;
	box.pmax.y = size.y * 0.5f;
	box.pmax.z = size.z * 0.5f;

	box.pmin.x = -box.pmax.x;
	box.pmin.y = -box.pmax.y;
	box.pmin.z = -box.pmax.z;

	pt[0] = box[0];
	pt[1] = box[1];
	pt[2] = box[3];
	pt[3] = box[2];
	gw->polyline(4, pt, NULL, NULL, 1, NULL);
	pt[0] = box[4];
	pt[1] = box[5];
	pt[2] = box[7];
	pt[3] = box[6];
	gw->polyline(4, pt, NULL, NULL, 1, NULL);
	pt[0] = box[0];
	pt[1] = box[4];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);
	pt[0] = box[1];
	pt[1] = box[5];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);
	pt[0] = box[2];
	pt[1] = box[6];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);
	pt[0] = box[3];
	pt[1] = box[7];
	gw->polyline(2, pt, NULL, NULL, 0, NULL);
}

void ForceViewer::DrawVectors(GraphicsWindow* gw)
{
	static Point3 pts[2];

	gw->startSegments();
	int count = m_samplePoints.Count();
	for (int ii=0; ii<count; ++ii)
	{
		pts[0] = m_samplePoints[ii];
		pts[1] = m_samplePoints[ii] + m_sampleForces[ii];
		gw->segment(pts, TRUE);
	}
	gw->endSegments();
}

void ForceViewer::DrawVectorBases(ViewExp* vpt, GraphicsWindow* gw, Matrix3 objectToWorld)
{
	Matrix3 viewToWorld;
	vpt->GetAffineTM(viewToWorld);
	viewToWorld = Inverse(viewToWorld);
	Point3 viewZ = viewToWorld.GetRow(2);

	int count = m_samplePoints.Count();
	for (int ii=0; ii<count; ++ii)
	{
		Point3 vecZ = objectToWorld.VectorTransform(m_sampleForces[ii]);

		if (DotProd(vecZ, viewZ) >= -0.000001f)
			gw->marker(&m_samplePoints[ii], SM_DOT_MRKR);
		else
			gw->marker(&m_samplePoints[ii], SM_CIRCLE_MRKR);
	}
}

void ForceViewer::DrawLattice(GraphicsWindow* gw)
{
	Point3 d(
		size.x/float(res.x),
		size.y/float(res.y),
		size.z/float(res.z)
	);

	Point3 min, max;
	max.x = size.x * 0.5f;
	min.x = -max.x;
	max.y = size.y * 0.5f;
	min.y = -max.y;
	max.z = size.z * 0.5f;
	min.z = -max.z;

	Point3 pt[3];
	float x, y, z;
	int ix, iy, iz;

	z = min.z;
	for (iz=0; iz<=res.z; iz++) {
		pt[0].z = z;
		pt[1].z = z;

		x = min.x;
		pt[0].y = min.y;
		pt[1].y = max.y;
		for (ix=0; ix<=res.x; ix++) {
			pt[0].x = x;
			pt[1].x = x;
			gw->polyline(2, pt, NULL, NULL, 0, NULL);
			x += d.x;
		}

		y = min.y;
		pt[0].x = min.x;
		pt[1].x = max.x;
		for (iy=0; iy<=res.y; iy++) {
			pt[0].y = y;
			pt[1].y = y;
			gw->polyline(2, pt, NULL, NULL, 0, NULL);
			y += d.y;
		}

		z += d.z;
	}

	pt[0].z = min.z;
	pt[1].z = max.z;
	y = min.y;
	for (iy=0; iy<=res.y; iy++) {
		pt[0].y = y;
		pt[1].y = y;
		x = min.x;
		for (ix=0; ix<=res.x; ix++) {
			pt[0].x = x;
			pt[1].x = x;
			gw->polyline(2, pt, NULL, NULL, 0, NULL);
			x += d.x;
		}
		y += d.y;
	}
}
