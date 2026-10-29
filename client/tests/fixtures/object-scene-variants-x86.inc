// Referencia x86 de variantes CreateObject, IDA 0x004FF5A0, base 7bb5ec15.

    if (SceneFlag == 2 || SceneFlag == 4) {
        switch (param_1) {
        case 0x3c:
            puVar3[3]=0x3f4ccccd; puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3e4ccccd;
            *(BYTE*)(puVar3+0x37)=1; break;
        case 0xa0:
            puVar3[3]=0x3d343958; puVar3[0x19]=1; puVar3[0x1a]=0x3f800000;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000;
            *(BYTE*)(puVar3+0x37)=0; break;
        case 0xa1:
            puVar3[3]=0x3f4ccccd; puVar3[0x19]=0; puVar3[0x1a]=0x3f800000;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000; break;
        case 0xa2:
            
            
            
            
            puVar3[3]=0x3f19999a; puVar3[0x19]=1;
            *(BYTE*)(puVar3+0x37)=0; break;
        case 0xa3:
            puVar3[3]=0x40400000; puVar3[0x19]=0;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f000000; break;
        case 0xa4:
            puVar3[0x19]=10;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000; break;
        }
    }

    
    
    switch (gMapManager.GetCurrentMap()) {
    case 0:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 6: case 0x91: case 0x92: break;
        case 0x34: puVar3[0x19]=1; return puVar3;
        case 0x3b: puVar3[0x33]=0; return puVar3;
        case 0x50: puVar3[0x34]=0xc2480000; return puVar3;
        case 0x5a: case 0x96: puVar3[0x19]=1; [[fallthrough]];
        case 0x60: case 0x61: puVar3[0x33]=0x3e99999a; return puVar3;
        case 0x62: case 0x77: puVar3[0x19]=2; return puVar3;
        case 0x69: puVar3[0x19]=3; return puVar3;
        case 0x75: case 0x7a: puVar3[0x19]=4; return puVar3;
        case 0x76: puVar3[0x19]=8; return puVar3;
        case 0x85:
            Entity_InitRenderState(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe; return puVar3;
        }
        break;
    case 1:
        if (param_1==0x3b) break;
        if (param_1==0x3c) {
            Entity_InitRenderState(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe;
        }
        return puVar3;
    case 2:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 0x13: case 0x5c: case 0x5d: puVar3[0x19]=0; return puVar3;
        case 0x14: case 0x41: case 0x56: case 0x58:
            
            
            
            
            
            
            
            
            puVar3[0xb]=puVar3[8]; puVar3[0xf]=puVar3[6]; puVar3[0xd]=puVar3[4];
            puVar3[0xa]=puVar3[7]; puVar3[0xe]=puVar3[5];
            { float angZ = *(float*)&puVar3[9];
              float fa = (float)(long long)(((long long)angZ) % 360);
              puVar3[9]=*(UINT*)&fa; puVar3[0xc]=*(UINT*)&fa; }
            return puVar3;
        case 0x16: case 0x19: case 0x28: case 0x2d: case 0x37: case 0x49: break;
        case 0x36: case 0x38: puVar3[0x19]=1; return puVar3;
        case 0x4e: puVar3[0x19]=3; return puVar3;
        case 0x5b:
            Entity_InitRenderState(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe; return puVar3;
        case 100: puVar3[0x16]=0xfffffffe; return puVar3;
        }
        
        
        
        
        
        break;
    case 3:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 1: puVar3[0x19]=1; return puVar3;
        case 8: break;
        case 9: puVar3[0x19]=3; return puVar3;
        case 0x11: case 0x13: case 0x25: puVar3[0x19]=0; return puVar3;
        case 0x12: puVar3[0x19]=2; return puVar3;
        case 0x26: Entity_InitRenderState(puVar3); puVar3[0x16]=0xfffffffe; return puVar3;
        }
        
        
        break;
    default:
        goto lbl_skip_init;
    case 7:
        if (param_1==0x27) { Entity_InitRenderState(puVar3); puVar3[0x16]=0xfffffffe; return puVar3; }
        return puVar3;
    case 8:
        if (param_1==0x4e) { Entity_InitRenderState(puVar3); goto lbl_skip_init; }
        return puVar3;
    }
    Entity_InitRenderState(puVar3);
lbl_skip_init:
    return puVar3;