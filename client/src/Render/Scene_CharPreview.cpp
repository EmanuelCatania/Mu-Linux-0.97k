// Scene_CharPreview.cpp
// 3D character preview rendering during login and character selection screens.
//
// RenderInfomation3D @ 0x0051e0c0 - Scene_CharPreview
// FUN_005239a0 @ 0x005239a0 - UI_DrawLoginBg

#include "stdafx.h"
#include "Render/Gfx.h"

// IDA: RenderInfomation3D (0x0051E0C0)
void RenderInfomation3D(void)
{
  float aspect;
  short itemType;
  byte option1;
  uint level;

  if (DAT_083a7c24 == 0x97) {
    if (DAT_07eaa13c != 1) {
      return;
    }
  }
  else if (DAT_083a7c24 != 0x99) {
    return;
  }

  Gfx::MatrixMode(Gfx::Matrix::Projection);
  Gfx::PushMatrix();
  Gfx::LoadIdentity();
  GL_SetViewport(0, 0, gWindow.GetRealWidth(), gWindow.GetHeight());
  aspect = (float)((double)gWindow.GetRealWidth() / (double)(int)gWindow.GetHeight());
  GL_SetPerspective(0x3f800000, aspect, (int)DAT_0056154c, DAT_00561550);

  Gfx::MatrixMode(Gfx::Matrix::ModelView);
  Gfx::PushMatrix();
  Gfx::LoadIdentity();
  GL_GetModelViewMatrix((unsigned int *)&CameraMatrix);
  GL_EnableDepthTest();
  GL_EnableDepthWrites();

  level = DAT_07e91354;
  itemType = *(short*)DAT_07e91350;
  option1 = DAT_07e9136b;
  if (DAT_083a7c24 == 0x99) {
    level = DAT_07ea5244;
    itemType = *(short*)DAT_07ea5240;
    option1 = DAT_07ea525b;
  }

  RenderItem3D(245.0f, 115.0f, 40.0f, 60.0f, (int)itemType, level, option1, 0, '\x01');

  Gfx::PopMatrix();
  Gfx::MatrixMode(Gfx::Matrix::Projection);
  Gfx::PopMatrix();
  Gfx::MatrixMode(Gfx::Matrix::ModelView);
}

void FUN_005239a0(void)
{
  GL_SetBlendSrcOver('\x01');
  Gfx::Color3f(1.0f, 1.0f, 1.0f);
  // DESVIACION (Fase 4, pantalla ancha; DLL WRenderInterfaceEdge): el marco se
  // pega a los bordes de la ventana (`s` unidades de más a cada lado) y las
  // barras de arriba y de abajo se estiran para cerrar el hueco.  Con 4:3, s = 0.
  const float s = (float)gWindow.GetRightShift();
  GL_DrawTexture(0xc,320.0,0.0,256.0f + s,86.0,0.0,0.0,1.0,0.671875,'','');
  GL_DrawTexture(0xc,64.0f - s,0.0,256.0f + s,86.0,1.0,0.0,-1.0,0.671875,'','');
  GL_DrawTexture(0xd,576.0f + s,0.0,64.0,256.0,0.0,0.0,1.0,1.0,'','');
  GL_DrawTexture(0xd,-s,0.0,64.0,256.0,1.0,0.0,-1.0,1.0,'','');
  GL_DrawTexture(0xe,576.0f + s,256.0,64.0,224.0,0.0,0.0,1.0,0.875,'','');
  GL_DrawTexture(0xe,-s,256.0,64.0,224.0,1.0,0.0,-1.0,0.875,'','');
  GL_DrawTexture(0xf,320.0,448.0,256.0f + s,32.0,0.0,0.0,1.0,1.0,'','');
  GL_DrawTexture(0xf,64.0f - s,448.0,256.0f + s,32.0,1.0,0.0,-1.0,1.0,'','');
}
