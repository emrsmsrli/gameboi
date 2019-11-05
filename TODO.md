# TODO

- integer promotion is undefined behaviour, use casts
- try to organize `cpu::decode`
- implement templated `AddressRange`
- interrupts should enable-disable after one instruction is executed.
- reduce branching please
- return pairs instead of passing out params

## Major
- todo modify compiler specific warnings 
  - https://clang.llvm.org/docs/DiagnosticsReference.html

## Minor
-  organise all includes

## PPU notes

RGB Translation by CGBs
When developing graphics on PCs, note that the RGB values will have different appearance on CGB displays as on VGA monitors:
The highest intensity will produce Light Gray color rather than White. The intensities are not linear; the values 10h-1Fh will all appear very bright, while medium and darker colors are ranged at 00h-0Fh.
The CGB display will mix colors quite oddly, increasing intensity of only one R,G,B color will also influence the other two R,G,B colors.
For example, a color setting of 03EFh (Blue=0, Green=1Fh, Red=0Fh) will appear as Neon Green on VGA displays, but on the CGB it'll produce a decently washed out Yellow.

RGB Translation by GBAs
Even though GBA is described to be compatible to CGB games, most CGB games are completely unplayable on GBAs because most colors are invisible (black). Of course, colors such like Black and White will appear the same on both CGB and GBA, but medium intensities are arranged completely different.
Intensities in range 00h..0Fh are invisible/black (unless eventually under best sunlight circumstances, and when gazing at the screen under obscure viewing angles), unfortunately, these intensities are regulary used by most existing CGB games for medium and darker colors.
Newer CGB games may avoid this effect by changing palette data when detecting GBA hardware. A relative simple method would be using the formula GBA=CGB/2+10h for each R,G,B intensity, probably the result won't be perfect, and (once colors became visible) it may turn out that the color mixing is different also, anyways, it'd be still ways better than no conversion.
Asides, this translation method should have been VERY easy to implement in GBA hardware directly, even though Nintendo obviously failed to do so. How did they say, This seal is your assurance for excellence in workmanship and so on?


