# Sonidos de los emotes (`Data/Sound/Dances/`)

Esta carpeta **no se distribuye con el cliente** y esta en `.gitignore`: los audios de los
emotes de Fortnite son de Epic Games y no tenemos derecho a redistribuirlos. Sin ellos el juego
funciona igual: el emote se ve, pero no suena.

## Como agregarlos

Copiar los archivos `.mp3` en esta carpeta (`Data/Sound/Dances/`), con exactamente estos nombres
(no importan las mayusculas). Cada emote usa el archivo de la tabla de abajo; los que tienen varios
candidatos (`a|b`) eligen uno al azar entre los que existan, y los de `-` no tienen sonido.

- Se reproducen en el cliente (no pasan por el servidor): cada jugador oye los emotes de los demas
  si tiene los archivos, con volumen que baja con la distancia (completo a menos de 8 casillas,
  silencio a mas de 25).
- Volumen: 30 % del nivel de efectos de `Config.ini` (`[Sound] SoundLevel`).
- Si el emote se cancela caminando o termina, el sonido se corta. Los emotes con loop repiten el audio.
- El baile clasico (`DanceMoves`, `/fortnite 0`) usa siempre `ninja_dance_01`.

## Tabla (numero de `/fortnite N`, emote, archivo)

| N | Emote | Archivo(s) `.mp3` |
|---|---|---|
| 0 | DanceMoves | ninja_dance_01 |
| 1 | Emote_Mask_Off_Loop | hip_hop_good_vibes_mix_01_loop |
| 2 | Emote_Zippy_Dance | emote_zippy_a |
| 3 | ElectroShuffle | athena_emote_electroshuffle_music |
| 4 | Emote_AerobicChamp | emote_aerobics_01 |
| 5 | Emote_Bendy | athena_music_emotes_bendy |
| 6 | Emote_BandOfTheFort | athena_emote_bandofthefort_music |
| 7 | Emote_Boogie_Down | emote_boogiedown |
| 8 | Emote_Capoeira | emote_capoeira |
| 9 | Emote_Charleston | athena_emote_flapper_music |
| 10 | Emote_Chicken | athena_emote_chicken_foley_01 |
| 11 | Emote_Dance_Shoot | athena_emotes_music_shoot_v7 |
| 12 | Emote_Dance_SwipeIt | athena_emotes_music_swipeit |
| 13 | Emote_Dance_Disco_T3 | athena_emote_disco |
| 14 | Emote_DG_Disco | athena_emote_disco |
| 15 | Emote_Dance_Worm | athena_emote_worm_music |
| 16 | Emote_Dance_Loser | athena_music_emotes_takethel |
| 17 | Emote_Dance_Breakdance | athena_emote_breakdance_music |
| 18 | Emote_Dance_Pump | emote_groove_jam_a |
| 19 | Emote_Dance_RideThePony | athena_emote_ridethepony_music_01 |
| 20 | Emote_Dab | - |
| 21 | Emote_EasternBloc | eastern_bloc_musc_setup_d |
| 22 | Emote_FancyFeet | athena_emotes_lankylegs_loop_02 |
| 23 | Emote_FlossDance | athena_emote_floss_music |
| 24 | Emote_FlippnSexy | emote_flippnsexy |
| 25 | Emote_Fresh | athena_emote_fresh_music |
| 26 | Emote_GrooveJam | emote_groove_jam_a |
| 27 | Emote_guitar | br_emote_shred_guitar_mix_03_loop |
| 28 | Emote_Hillbilly_Shuffle | emote_hillbilly_shuffle |
| 29 | Emote_Hiphop_01 | s5_hiphop_breakin_132bmp_loop |
| 30 | Emote_Hip_Hop | s5_hiphop_breakin_132bmp_loop |
| 31 | Emote_Hula | emote_hula_01 |
| 32 | Emote_InfiniDab_Loop | athena_emote_infinidab |
| 33 | Emote_Intensity_Loop | emote_intensity |
| 34 | Emote_IrishJig | emote_irish_jig_foley_music_loop |
| 35 | Emote_KoreanEagle | athena_music_emotes_koreaneagle |
| 36 | Emote_Kpop_02 | emote_kpop_01 |
| 37 | Emote_LivingLarge | emote_livinglarge_a |
| 38 | Emote_Maracas | emote_samba_new_b |
| 39 | Emote_PopLock | athena_emote_poplock |
| 40 | Emote_PopRock | emote_poprock_01 |
| 41 | Emote_RobotDance | athena_emote_robot_music |
| 42 | Emote_T-Rex | emote_dino_complete |
| 43 | Emote_TechnoZombie | athena_emote_founders_music |
| 44 | Emote_Twist | athena_emotes_music_twist |
| 45 | Emote_WarehouseDance_Loop | emote_warehouse |
| 46 | Emote_Wiggle | wiggle_music_loop |
| 47 | Emote_Youre_Awesome | youre_awesome_emote_music |
| 48 | Emote_Fonzie_Pistol | - |
| 49 | Emote_Bring_It_On | - |
| 50 | Emote_ThumbsDown | - |
| 51 | Emote_ThumbsUp | - |
| 52 | Emote_BlowKiss | blowkiss |
| 53 | Emote_Calculated | - |
| 54 | Emote_Confused | - |
| 55 | Emote_Chug | gogogo |
| 56 | Emote_Cry | bananacry |
| 57 | Emote_DustingOffHands | - |
| 58 | Emote_DustOffShoulders | athena_emote_hot_music |
| 59 | Emote_Facepalm | athena_emote_facepalm_foley_01 |
| 60 | Emote_Fishing | fishing |
| 61 | Emote_Flex | flexing |
| 62 | Emote_golfclap | clapping |
| 63 | Emote_HandSignals | - |
| 64 | Emote_HeelClick | emote_heelclick |
| 65 | Emote_Hotstuff | emote_hotstuff |
| 66 | Emote_IBreakYou | - |
| 67 | Emote_IHeartYou | iheartyou |
| 68 | Emote_Kung-Fu_Salute | - |
| 69 | Emote_Laugh | emote_laugh_01 |
| 70 | Emote_Luchador | emote_luchador |
| 71 | Emote_Make_It_Rain | athena_emote_makeitrain_music |
| 72 | Emote_NotToday | - |
| 73 | Emote_RockPaperScissor_Paper | - |
| 74 | Emote_RockPaperScissor_Rock | - |
| 75 | Emote_RockPaperScissor_Scissor | - |
| 76 | Emote_Salt | - |
| 77 | Emote_Salute | athena_emote_salute_foley_01 |
| 78 | Emote_SmoothDrive | dropit |
| 79 | Emote_Snap | emote_snap1 |
| 80 | Emote_StageBow | emote_stagebow |
| 81 | Emote_Wave2 | - |
| 82 | Emote_Yeet | emote_yeet |
| 83 | Emote_Smooth_Moves | smooth_moves |
| 84 | Emote_Friday13 | california_girls |
| 85 | Emote_Thanos_Twerk | thanos_twerk |
| 86 | Emote_Gangnam_Style | psychic |
| 87 | Emote_InDaGhetto | emote_vivid |
| 88 | Emote_BlindingLights | emote_autumn_tea |
| 89 | Emote_Griddy | emote_griddles_music |
| 90 | Emote_ILikeToMoveIt | emote_jumpingjoy |
| 91 | Emote_Macarena | emote_macaroon_music |
| 92 | Emote_NeverGonna | emote_nevergonna |
| 93 | Emote_NinjaStyle | emote_tour_bus |
| 94 | Emote_PumpkinDance | pumpkin_dance |
| 95 | Emote_PumpUpTheJam | deflated_emote_music |
| 96 | Emote_Renegade | emote_just_home_music |
| 97 | Emote_RushinAround | emote_comrade |
| 98 | Emote_SaySo | emote_hotpink |
| 99 | Emote_Stuck | emote_downward |
| 100 | Emote_ToosieSlide | emote_art_giant |
| 101 | Emote_AirShredder | air_guitar_emote |
| 102 | Emote_Crossbounce | emote_blaster |
| 103 | Emote_DistractionDance | distraction |
| 104 | Emote_Headbanger | headbanger_music |
| 105 | Emote_HitchHiker | hitchhiker_music |
| 106 | Emote_ItsGoTime | itsgotime_music |
| 107 | Emote_KneeSlapper | kneeslapper_music |
| 108 | Emote_Showstopper | showstopper_music |
| 109 | Emote_Sprinkler | sprinkler_music |
| 110 | Emote_Gmod | gmod_select |
| 111 | Emote_Cena | cant_c_me |
| 112 | Emote_ChickenDance | pollo_dance |
| 113 | Emote_Ghostbusters | toastbust |
| 114 | Emote_Lebron | lebronjame |
| 115 | Emote_Martian | leave_the_door |
| 116 | Emote_RememberMe_Loop | unforgettable |
| 117 | Emote_Rollie | rollie_rollie |
| 118 | Emote_Scenario | scenariooo |
