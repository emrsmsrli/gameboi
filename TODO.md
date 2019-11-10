# TODO

- try to organize `cpu::decode`
- interrupts should enable-disable after one instruction is executed.
- reduce branching please
- return pairs instead of passing out params
- scale out color values from 0x00-0x1F range to 0x00-0xFF range

## Major
- todo modify compiler specific warnings 
  - https://clang.llvm.org/docs/DiagnosticsReference.html
- implement mbc5

## Minor
- organise all includes

## PPU notes

RGB Translation by CGBs
When developing graphics on PCs, note that the RGB values will have different appearance on CGB displays as on VGA monitors:
The highest intensity will produce Light Gray color rather than White. The intensities are not linear; the values 10h-1Fh will all appear very bright, while medium and darker colors are ranged at 00h-0Fh.
The CGB display will mix colors quite oddly, increasing intensity of only one R,G,B color will also influence the other two R,G,B colors.
For example, a color setting of 03EFh (Blue=0, Green=1Fh, Red=0Fh) will appear as Neon Green on VGA displays, but on the CGB it'll produce a decently washed out Yellow.
