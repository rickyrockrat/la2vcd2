CVS log file, readme for sourceforge.
In 1.11, changed API like so:

1) vcd_add_file(l,nul,16,bits) to vcd_add_file(l,nul,16,bits,SIGNAL_TYPE)
2)  vcd_add_signal(firstsig,lastsig, lastinfile,signame,lsb,msb) to 
     vcd_add_signal(firstsig,SIGNAL_TYPE, lastsig, lastinfile,signame,lsb,msb) where
SIGNAL_TYPE is either V_WIRE or V_REAL.

V_REAL is intended to work with scope data. Not sure it's working, though it does seem to import into GTKWave OK....Data seems a little funky.

Working file: la2vcd.c
head: 1.12
symbolic names:
        V0_03L1: 1.11
        V_0_0_2L6: 1.10
        V_0_0_2L5: 1.10
        V_0_0_2L3: 1.9
        V_0_0_2L2: 1.9
        V_0_0_2L1: 1.8
        V_0_0_2L: 1.7
        V0_02F: 1.3
        V0_02: 1.1
revision 1.12
date: 2011-12-07 00:35:09 -0700;  author: dfs;  state: Exp;  lines: +1 -1;  commitid: VB8XHMxODcaTeeKv;
Added extern for _vcd_add_signal
----------------------------
revision 1.11
date: 2011-10-10 22:40:13 -0600;  author: dfs;  state: Exp;  lines: +3 -5;  commitid: lezhWQRhza2t6TCv;
Added real data var
----------------------------
revision 1.10
date: 2010-01-19 14:22:12 -0700;  author: dfs;  state: Exp;  lines: +1 -0;  commitid: g18suv7bGkBJT0ku;
Fixed VERSION issue
----------------------------
revision 1.9
date: 2010-01-17 16:12:51 -0700;  author: dfs;  state: Exp;  lines: +5 -9;  commitid: QgxlnNsJAFeGzLju;
Added vcd_add_file, close_la2vcd, vcd_read_sample, and fixed up split_field vcd_add_signal
----------------------------
revision 1.8
date: 2010-01-17 08:38:32 -0700;  author: dfs;  state: Exp;  lines: +15 -2;  commitid: 4AAjBwMjBnYO3Jju;
Moved fatal to here, added new err handling from lib
----------------------------
revision 1.7
date: 2010-01-17 06:20:59 -0700;  author: dfs;  state: Exp;  lines: +5 -1;  commitid: m8z6QuNfsQDDiIju;
Added GPL, comments
----------------------------
revision 1.6
date: 2010-01-17 06:11:59 -0700;  author: dfs;  state: Exp;  lines: +7 -695;  commitid: ZtETfHfbVW3yfIju;
Moved core to lib
----------------------------
revision 1.5
date: 2010-01-16 16:16:40 -0700;  author: dfs;  state: Exp;  lines: +2 -2;  commitid: ehDcLYuhHh5ZCDju;
Changed Version to .02F
----------------------------
revision 1.4
date: 2010-01-16 15:40:52 -0700;  author: dfs;  state: Exp;  lines: +507 -362;  commitid: DTJXdbLuEGHHqDju;
Initial start at lib interface...first working copy
----------------------------
revision 1.3
date: 2010-01-13 11:22:46 -0700;  author: dfs;  state: Exp;  lines: +13 -0;  commitid: Xcya4ykSCKH86eju;
Added comments on bug fixes
----------------------------
revision 1.2
date: 2010-01-13 11:20:32 -0700;  author: dfs;  state: Exp;  lines: +28 -26;  commitid: eCG8C7zHg9DL4eju;
Added debug prints and fixed 2 bugs:
1) Signal splitting didn't get the MSB of the signal list (5-3 got 4-3)
2) If bits are >32, you get this error:
"fatal error: field bit positions out of range"
----------------------------
revision 1.1
date: 2010-01-13 11:18:06 -0700;  author: dfs;  state: Exp;  commitid: 10SAG8yLt6tx4eju;
Version 0.02
=============================================================================
