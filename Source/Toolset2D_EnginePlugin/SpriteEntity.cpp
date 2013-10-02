//=======
//
// Author: Joel Van Eenwyk
// Purpose: Handle sprite setup and rendering
// TODO:
//   * Add shader support
//   * Add 3D mode more it just generates a plane
//   * Add alpha test option
//   * Add blending mode options (additive, blend, etc)
//
//=======
//
//       .-""""""-.
//     .'          '.
//    /   O    -=-   \
//   :                :
//   |                | 
//   : ',          ,' :
//    \  '-......-'  /
//     '.          .'
//       '-......-'
//

#include "Toolset2D_EnginePluginPCH.h"

#include "SpriteEntity.hpp"
#include "SpriteManager.hpp"

#include <Vision/Runtime/EnginePlugins/EnginePluginsImport.hpp>
#include <Vision/Runtime/Base/ThirdParty/tinyXML/tinyxml.h>

#define CURRENT_SPRITE_VERSION 2

#define PROP_TEXTURE_FILENAME TextureFilename
#define PROP_TEXTURE_SCALE TextureScale

V_IMPLEMENT_SERIAL(Sprite, VisBaseEntity_cl, 0, &gToolset2D_EngineModule);

// Called by the engine when entity is created. Not when it is de-serialized!
void Sprite::InitFunction()
{
	VisBaseEntity_cl::InitFunction();

	Clear();
	CommonInit();
}

// called by the engine when entity is destroyed
void Sprite::DeInitFunction()
{
	VisBaseEntity_cl::DeInitFunction();
	CommonDeInit();
}

// called by our InitFunction and our de-serialization code
void Sprite::CommonInit()
{
	SpriteManager::GlobalManager().AddSprite(this);
	SetShoeBoxData(m_spriteSheetFilename, m_xmlDataFilename);
}

void Sprite::CommonDeInit()
{ 
	SpriteManager::GlobalManager().RemoveSprite(this);	
	Clear();
}

void Sprite::Clear()
{
	m_loaded = false;

	Fullscreen = false;
	ScrollSpeed.setZero();

	m_currentState = -1;
	m_currentFrame = -1;
	m_frameTime = 0.f;
	m_paused = false;
	m_scrollOffset.setZero();

	ClearTextures();
}

void Sprite::ClearTextures()
{
	// Remove any old cells, states, or textures
	m_cells.RemoveAll();
	m_states.RemoveAll();
	m_stateNameToIndex.Reset();
	m_spTextureAnimation = NULL;
	V_SAFE_RELEASE(m_spSpriteSheetTexture);
}

bool Sprite::UpdateTextures()
{
	ClearTextures();

	bool success = false;

	m_spSpriteSheetTexture = Vision::TextureManager.Load2DTexture(m_spriteSheetFilename);
	if (m_spSpriteSheetTexture)
	{
		// Go ahead and add a reference to this texture
		m_spSpriteSheetTexture->AddRef();

		// TODO: Perhaps it's a bit weird to have a spritesheet that also has animations on it--all of the cells
		//       would have to be in the same places for each sheet, which is odd. Probably assert when there is
		//       XML document AND a texture animation.
		m_spTextureAnimation = Vision::TextureManager.GetAnimationInstance(m_spSpriteSheetTexture);

		TiXmlDocument xmlDocument;

		// If we successfully load the XML, then there is data we can parse about the sprites in the sheet
		if ( xmlDocument.LoadFile(m_xmlDataFilename, Vision::File.GetManager()) )
		{
			const char *szActionNode = "SubTexture";

			for (TiXmlElement *pNode = xmlDocument.RootElement()->FirstChildElement(szActionNode);
				pNode != NULL;
				pNode = pNode->NextSiblingElement(szActionNode) )
			{
				const char *name = pNode->Attribute("name");

				int x;
				int y;
				int width;
				int height;
				int ox;
				int oy;

				pNode->QueryIntAttribute("x", &x);
				pNode->QueryIntAttribute("y", &y);
				pNode->QueryIntAttribute("width", &width);
				pNode->QueryIntAttribute("height", &height);
				pNode->QueryIntAttribute("ox", &ox);
				pNode->QueryIntAttribute("oy", &oy);

				const int newCellIndex = m_cells.Append(SpriteCell());
				SpriteCell *currentCell = &m_cells[newCellIndex];
				currentCell->name = name;
				currentCell->offset.x = static_cast<float>(x);
				currentCell->offset.y = static_cast<float>(y);
				currentCell->pivot.x = static_cast<float>(ox);
				currentCell->pivot.y = static_cast<float>(oy);
				currentCell->width = width;
				currentCell->height = height;

				const char *result = strrchr(name, '_');
				int index = -1;
				if (result != NULL)
					index = result - name;

				SpriteState *state = NULL;
				int stateIndex = -1;
				if (index == -1)
				{
					const char *extension = strrchr(name, '.');
					int extensionIndex = -1;
					if (extension != NULL)
						extensionIndex = extension - name;
					VString stateName = VString(name, extensionIndex);

					stateIndex = m_states.Append(SpriteState());
					state = &m_states[stateIndex];
					state->name = stateName;
					state->framerate = 30.0f;
					m_stateNameToIndex.Set(state->name, stateIndex);
				}
				else
				{
					VString s = name;
					VString stateName = VString(name, index);
					VString last = VString(&name[index + 1], strlen(name) - index);
					VString number = VString(last.GetChar(), last.Find("."));
					currentCell->index = atoi(number);

					for (int i = 0; i < m_states.GetSize(); i++)
					{
						if (m_states[i].name == stateName)
						{
							stateIndex = i;
							state = &m_states[i];
							break;
						}
					}

					if (stateIndex == -1)
					{
						stateIndex = m_states.Append(SpriteState());
						state = &m_states[stateIndex];
						state->name = stateName;
						state->framerate = 10.f;
						m_stateNameToIndex.Set(state->name, stateIndex);
					}
				}

				state->cells.Append(newCellIndex);
			}

			success = true;
		}
		// No XML describing the sprite sheet, but we do have a sprite texture
		else if (m_spSpriteSheetTexture != NULL)
		{
			int stateIndex = m_states.Append(SpriteState());
			SpriteState *state = &m_states[stateIndex];
			state->name = m_spriteSheetFilename;
			state->framerate = 30.0f;
			state->cells.Add(0);
			m_stateNameToIndex.Set(state->name, stateIndex);

			const int newCellIndex = m_cells.Append(SpriteCell());
			SpriteCell *currentCell = &m_cells[newCellIndex];

			char buffer[FS_MAX_PATH];
			VFileHelper::GetFilenameNoExt(buffer, m_spriteSheetFilename);
			currentCell->name = buffer;
			currentCell->offset.x = 0.f;
			currentCell->offset.y = 0.f;
			currentCell->pivot.x = 0.f;
			currentCell->pivot.y = 0.f;
			currentCell->width = m_spSpriteSheetTexture->GetTextureWidth();
			currentCell->height = m_spSpriteSheetTexture->GetTextureHeight();

			m_currentState = m_currentFrame = 0;

			success = true;
		}

		if (m_states.GetSize() > 0)
		{
			m_currentState = m_currentFrame = 0;
		}
	}

	return success;
}

bool Sprite::SetShoeBoxData(const char *spriteSheetFilename, const char *xmlFilename)
{
	bool success = false;

	if (m_spriteSheetFilename == spriteSheetFilename &&
		m_xmlDataFilename == xmlFilename &&
		m_loaded)
	{
		success = true;
	}
	else
	{
		m_spriteSheetFilename = spriteSheetFilename;
		m_xmlDataFilename = xmlFilename;

		success = UpdateTextures();
		if (success)
		{
			m_loaded = true;
		}
	}

	return success;
}

const VArray<VString> Sprite::GetStateNames() const
{
	VArray<VString> names;

	for (int i = 0; i < m_states.GetSize(); i++)
	{
		names.Add(m_states[i].name);
	}

	return names;
}

const SpriteState *Sprite::GetCurrentState() const
{
	const SpriteState *state = NULL;
	if (m_currentState != -1)
	{
		state = &m_states[m_currentState];
	}
	return state;
}

int Sprite::GetCurrentFrame() const
{
	return m_currentFrame;
}

void Sprite::SetCurrentFrame(int currentFrame)
{
	if (m_currentState != -1)
	{
		m_paused = true;

		const SpriteState *s = &m_states[m_currentState];
		const int numCells = s->cells.GetSize();		
		m_currentFrame = hkvMath::clamp(currentFrame, 0, numCells - 1);
	}
}

void Sprite::ThinkFunction()
{
	if (m_currentState != -1 && !m_paused)
	{
		const float dt = Vision::GetTimer()->GetTimeDifference();

		m_frameTime += dt;
		m_scrollOffset += ScrollSpeed * dt;

		if (!hkvMath::isFloatEqual(ScrollSpeed.x, 0.0f))
		{
			if (ScrollSpeed.x > 0 && m_scrollOffset.x > 1.0f)
			{
				m_scrollOffset.x -= 1.0f;
			}
			else if (ScrollSpeed.x < 0 && m_scrollOffset.x < 0.0f)
			{
				m_scrollOffset.x += 1.0f;
			}
		}

		if (!hkvMath::isFloatEqual(ScrollSpeed.y, 0.0f))
		{
			if (ScrollSpeed.y > 0 && m_scrollOffset.y > 1.0f)
			{
				m_scrollOffset.y -= 1.0f;
			}
			else if (ScrollSpeed.y < 0 && m_scrollOffset.y < 0.0f)
			{
				m_scrollOffset.y += 1.0f;
			}
		}

		const SpriteState *state = &m_states[m_currentState];
		const float inverseFramerate = 1.0f / state->framerate;
		if (m_frameTime >= inverseFramerate)
		{
			const int numCells = state->cells.GetSize();
			m_currentFrame = (m_currentFrame + 1) % numCells;
			m_frameTime -= dt;
		}
	}
}

bool Sprite::SetState(const char *state)
{
	int index = m_stateNameToIndex.Find(state);
	if (index != -1)
	{
		m_currentState = index;
		m_currentFrame = 0;
		m_frameTime = 0.f;
	}
	return (index != -1);
}

void Sprite::SetFramePercent(float percent)
{
	if (m_currentState != -1)
	{
		m_paused = true;

		const SpriteState *s = &m_states[m_currentState];
		const int numCells = s->cells.GetSize();		
		m_currentFrame = static_cast<int>( hkvMath::clamp(percent, 0.f, 1.f) * static_cast<float>(numCells - 1) );
	}
}

void Sprite::Play()
{
	m_paused = false;
}

void Sprite::Pause()
{
	m_paused = true;
}

void Sprite::SetScrollSpeed(hkvVec2 scrollSpeed)
{
	ScrollSpeed = scrollSpeed;
}

const hkvVec2 &Sprite::GetScrollSpeed() const
{
	return ScrollSpeed;
}

void Sprite::SetFullscreenMode(bool enabled)
{
	Fullscreen = enabled;
}

bool Sprite::IsFullscreenMode() const
{
	return Fullscreen;
}

void Sprite::OnVariableValueChanged(VisVariable_cl *pVar, const char *value)
{
	if ( !strcmp(pVar->name, "TextureFilename") )
	{
		if (value &&
			value[0] &&
			m_spriteSheetFilename != value)
		{
			m_spriteSheetFilename = value;
			UpdateTextures();
		}
		else
		{
			ClearTextures();
		}
	}
}

BOOL Sprite::AddComponent(IVObjectComponent *pComponent)
{
	BOOL success = VisBaseEntity_cl::AddComponent(pComponent);
	return success;
}

void Sprite::DebugRender(IVRenderInterface *pRenderer, float fSize, VColorRef iColor, bool bRenderConnections) const
{
	//VSimpleRenderState_t state(VIS_TRANSP_ALPHA, RENDERSTATEFLAG_FRONTFACE);
	//hkvAlignedBBox bbox;
	//hkvVec3 vRad(fSize, fSize, fSize);
	//const hkvVec3 pos = GetPosition();
	//bbox.m_vMin = pos - vRad;
	//bbox.m_vMax = pos + vRad;
	//pRenderer->RenderAABox(bbox, iColor, state);
}

void Sprite::Render(IVRender2DInterface *pRender, VSimpleRenderState_t& state)
{
	hkvVec2 tl;
	hkvVec2 br;

	hkvVec3 pos = GetPosition();	
	VTextureObject *texture = m_spSpriteSheetTexture;
	
	if (m_spTextureAnimation)
	{
		texture = m_spTextureAnimation->GetCurrentFrame();
	}

	if (texture != NULL)
	{
		const float width = static_cast<float>(texture->GetTextureWidth());
		const float height = static_cast<float>(texture->GetTextureHeight());

		if (m_currentState != -1)
		{
			hkvVec2 topLeft;
			hkvVec2 bottomRight;

			const SpriteState *spriteState = &m_states[m_currentState];
			const SpriteCell *cell = &m_cells[spriteState->cells[m_currentFrame]];

			float w = (float)cell->width / width;
			float h = (float)cell->height / height;
			float x = cell->offset.x / width;
			float y = cell->offset.y / height;

			topLeft.x = x + m_scrollOffset.x;
			topLeft.y = y + m_scrollOffset.y;
			bottomRight.x = topLeft.x + w;
			bottomRight.y = topLeft.y + h;

			if (IsFullscreenMode())
			{
				tl.x = 0;
				tl.y = 0;

				int x, y, w, h;
				Vision::Contexts.GetMainRenderContext()->GetViewport(x, y, w, h);
				
				if (w - width > h - height)
				{
					br.x = static_cast<float>(w);
					br.y = static_cast<float>(w * height) / width;
				}
				else
				{
					br.y = static_cast<float>(h);
					br.x = static_cast<float>(width * h) / height;
				}

				topLeft.x += pos.x / width;
				bottomRight.x += pos.x / width;
				topLeft.y += pos.y / height;
				bottomRight.y += pos.y / height;

				pRender->DrawTexturedQuad( tl, br, texture, topLeft, bottomRight, V_RGBA_WHITE, state );
			}
			else
			{
				const hkvVec3 &scale = GetScaling();
				tl.x += cell->pivot.x;
				tl.y += cell->pivot.y;
				br.x = tl.x + cell->width * scale.x;
				br.y = tl.y + cell->height * scale.y;

				const float ww = br.x - tl.x;
				const float hh = br.y - tl.y;
				hkvVec2 offset(ww / 2.0f, hh / 2.0f);

				// offset it so that we rotate around the center of the shape
				tl -= offset;
				br -= offset;

				hkvVec2 tr = tl;
				tr.x += ww;

				hkvVec2 bl = br;
				bl.x -= ww;

				// rotate the corners
				hkvMat3 rotation;
				rotation.setRotationMatrixZ(m_vOrientation.z);
				tl = (rotation * tl.getAsVec3(0.0f)).getAsVec2();
				br = (rotation * br.getAsVec3(0.0f)).getAsVec2();
				tr = (rotation * tr.getAsVec3(0.0f)).getAsVec2();
				bl = (rotation * bl.getAsVec3(0.0f)).getAsVec2();

				// offset it back to the corner
				tl += pos.getAsVec2() + offset;
				br += pos.getAsVec2() + offset;
				tr += pos.getAsVec2() + offset;
				bl += pos.getAsVec2() + offset;
				Overlay2DVertex_t vertices[6];

				vertices[0].Set(tl.x, tl.y, topLeft.x, topLeft.y);
				vertices[1].Set(bl.x, bl.y, bottomRight.x - w, bottomRight.y);
				vertices[2].Set(tr.x, tr.y, topLeft.x + w, topLeft.y);

				vertices[3].Set(tr.x, tr.y, topLeft.x + w, topLeft.y);
				vertices[4].Set(bl.x, bl.y, bottomRight.x - w, bottomRight.y);
				vertices[5].Set(br.x, br.y, bottomRight.x, bottomRight.y);

				pRender->Draw2DBuffer(6, vertices, texture, state);
			}
		}
		else
		{
			br.x = tl.x + width;
			br.y = tl.y + height;

			pRender->DrawTexturedQuad( tl, br, texture, hkvVec2(0, 0), hkvVec2(1, 1), V_RGBA_WHITE, state );
		}
	}
}

void Sprite::Serialize(VArchive &ar)
{
	VisBaseEntity_cl::Serialize(ar);

	if (ar.IsLoading())
	{
		Clear();

		char spriteVersion;
		ar >> spriteVersion;
		VASSERT(spriteVersion <= CURRENT_SPRITE_VERSION);

		char spriteSheetBuffer[FS_MAX_PATH + 1];
		ar.ReadStringBinary(spriteSheetBuffer, FS_MAX_PATH);
		m_spriteSheetFilename = spriteSheetBuffer;

		char xmlFilenameBuffer[FS_MAX_PATH + 1];
		ar.ReadStringBinary(xmlFilenameBuffer, FS_MAX_PATH);
		m_xmlDataFilename = xmlFilenameBuffer;

		ar >> TextureScale;
		ar >> ScrollSpeed.x;
		ar >> ScrollSpeed.y;
		ar >> Fullscreen;
	} 
	else
	{
		ar << (char)CURRENT_SPRITE_VERSION;

		ar.WriteStringBinary(m_spriteSheetFilename);
		ar.WriteStringBinary(m_xmlDataFilename);

		ar << TextureScale;
		ar << ScrollSpeed.x;
		ar << ScrollSpeed.y;
		ar << Fullscreen;
	}
}

void Sprite::OnSerialized(VArchive &ar)
{
	VisBaseEntity_cl::OnSerialized(ar);

	CommonInit();
}

START_VAR_TABLE(Sprite, VisBaseEntity_cl, "Sprite", 0, "")
	DEFINE_VAR_STRING_CALLBACK(Sprite, TextureFilename, "Sprite sheet", "white.dds", DISPLAY_HINT_TEXTUREFILE, NULL);
	DEFINE_VAR_FLOAT_AND_NAME(Sprite, PROP_TEXTURE_SCALE, "Scale", "Scale of the sprite sheet (% of pixels)", "0", 0, "Clamp(0, 1)");
END_VAR_TABLE