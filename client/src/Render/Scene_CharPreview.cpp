// Scene_CharPreview.cpp
// 3D character preview rendering during login and character selection screens.
//
// FUN_0051e0c0 @ 0x0051e0c0 - Scene_CharPreview
// FUN_005239a0 @ 0x005239a0 - UI_DrawLoginBg

#include "stdafx.h"

void FUN_0051e0c0(void)
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

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  GL_SetViewport(0, 0, DAT_0056156c, DAT_00561570);
  aspect = (float)((double)DAT_0056156c / (double)(int)DAT_00561570);
  GL_SetPerspective(0x3f800000, aspect, (int)DAT_0056154c, DAT_00561550);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  GL_GetModelViewMatrix((unsigned int *)&DAT_083a4140);
  GL_EnableDepthTest();
  GL_EnableDepthWrites();

  level = DAT_07e91354;
  itemType = *(short*)DAT_07e91350;
  option1 = DAT_07e9136b;
  if (DAT_083a7c24 == 0x99) {
    level = DAT_07ea5244;
    itemType = (short)DAT_07ea5240;
    option1 = DAT_07ea525b;
  }

  FUN_004e1be0(245.0f, 115.0f, 40.0f, 60.0f, (int)itemType, level, option1, '\x01');

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void FUN_005239a0(void)
{
  GL_SetBlendSrcOver('\x01');
  glColor3f(1.0f, 1.0f, 1.0f);
  GL_DrawTexture(0xc,320.0,0.0,256.0,86.0,0.0,0.0,1.0,0.671875,'\x01','\x01');
  GL_DrawTexture(0xc,64.0,0.0,256.0,86.0,1.0,0.0,-1.0,0.671875,'\x01','\x01');
  GL_DrawTexture(0xd,576.0,0.0,64.0,256.0,0.0,0.0,1.0,1.0,'\x01','\x01');
  GL_DrawTexture(0xd,0.0,0.0,64.0,256.0,1.0,0.0,-1.0,1.0,'\x01','\x01');
  GL_DrawTexture(0xe,576.0,256.0,64.0,224.0,0.0,0.0,1.0,0.875,'\x01','\x01');
  GL_DrawTexture(0xe,0.0,256.0,64.0,224.0,1.0,0.0,-1.0,0.875,'\x01','\x01');
  GL_DrawTexture(0xf,320.0,448.0,256.0,32.0,0.0,0.0,1.0,1.0,'\x01','\x01');
  GL_DrawTexture(0xf,64.0,448.0,256.0,32.0,1.0,0.0,-1.0,1.0,'\x01','\x01');
}
