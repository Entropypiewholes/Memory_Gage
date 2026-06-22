Icon assets go here (10x10 1-bit PNGs).

For v1 we intentionally omitted fap_icon / fap_icon_assets in application.fam so no build dependency on a binary PNG.

To add an icon later:
1. Create digimon_memory_10px.png (exactly 10x10, 1-bit / black & white, e.g. a simple gauge or "DM" symbol).
2. Update application.fam to include:
   fap_icon="digimon_memory_10px.png",
   fap_icon_assets="images",
3. Rebuild with ufbt.

The build system will generate digimon_memory_icons.h automatically.
