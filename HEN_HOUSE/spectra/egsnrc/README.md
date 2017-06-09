## EGSnrc spectra

All files in this directory are part of EGSnrc.

Some data files require strict formatting to work properly within EGSnrc, hence
it is not possible to insert a licence notice inside those files. They
are nevertheless distributed under the same terms as EGSnrc, and as a reminder a
copy of the LICENCE is included in this directory.

### Description

The spectra in this directory are based on spectra measured or calculated at
NRC, plus several from the literature. These files are readable by
ensrc.mortran, version 5 or later. The spectra are normalized to 1 count in the
spectrum. The "in-air" IR-192 spectra are on transverse axis at 100 cm from the source
(distance independent past a few centimetres). For a complete description and further references, see

A. Kosunen and D.W.O. Rogers, *Beam quality specification for photon beam
dosimetry*, [**Med. Phys. 20,** 1181-1188 (1993)](http://dx.doi.org/10.1118/1.598763).

J. Borg and D. W. O. Rogers, *Spectra and Air-Kerma Strength for Encapsulated
192-Ir Sources*, [**Med. Phys. 26,** 2441-2444 (1999).](http://dx.doi.org/10.1118/1.598763)

The following spectra are available.

| File name                          | Notes
|------------------------------------|-----------------------------------------------------------
|  10mvalnrcm.spectrum               | 10 MeV Al-target NRC measured (rev, norm 1)
|  10mvalnrcmfilt.spectrum           | 10  MV + Al-target + 14cm Al (/MeV) NRC measured + calc
|  10mvpbnrcm.spectrum               | 10 MeV Pb-target NRC measured (rev, norm 1)
|  15mvalnrcm.spectrum               | 15 MeV Al-target NRC measured (rev, norm 1)
|  15mvbenrcm.spectrum               | 15 MeV Be-target NRC measured (rev, norm 1)
|  15mvpbnrcm.spectrum               | 15 MeV Pb-target NRC measured (rev, norm 1)
|  20mvalnrcm.spectrum               | 20 MeV Al-target NRC measured (rev, norm 1)
|  20mvalnrcmfilt.spectrum           | 20 MV + Al-target measured + 14cm Al (calc)
|  20mvpbnrcm.spectrum               | 20 MeV Pb-target NRC measured (rev, norm 1)
|  250.spectrum                      | 250 kV spectrum from Len
|  25mvalnrcm.spectrum               | 25 MeV Al-target NRC measured (rev, norm 1
|  25mvpbnrcm.spectrum               | 25 MeV Pb-target NRC measured (rev, norm 1)
|  30mvalnrcm.spectrum               | 30 MeV Al-target NRC measured (rev, norm 1)
|  30mvpbnrcm.spectrum               | 30 MeV Pb-target NRC measured (rev, norm 1)
|  al10mev.spectrum                  | 10 MeV Al Schiff thin target
|  al20mev.spectrum                  | 20 MeV Al Schiff-thin target
|  al30mev.spectrum                  | 30 MeV Al thin target
|  co60.spectrum                     | 60-Co  spectrum source capsule and collimators Rogers et al 1987
|  cs137.spectrum                    | 137-Cs spectrum  (from NIST paper?)
|  mohan10.spectrum                  | Mohan et al 10 MV spectrum: counts/bin or counts/MeV
|  mohan15.spectrum                  | Mohan et al 15 MV spectrum: counts/bin or counts/MeV
|  mohan24.spectrum                  | Mohan et al 24 MV spectrum: counts/bin or counts/MeV
|  mohan4.spectrum                   | Mohan et al  4 MV spectrum: counts/bin or counts/MeV
|  mohan6.spectrum                   | Mohan et al  6 MV spectrum: counts/bin or counts/MeV
|  nrc10mv.spectrum                  | NRC standard 10.00 MeV on Al + Al filter (central)
|  nrc20mv.spectrum                  | NRC standard 20.00 MeV on Al + Al filter (central)
|  rt30.spectrum                     | 30 MeV beam for MSKCC racetrack microtron  (central)
|  rt50.spectrum                     | 50 MeV beam for MSKCC racetrack microtron  (central)
|  Ir192_VariSource.spectrum         | in air
|  Ir192_VariSource.spectrum         | in air
|  Ir192_alpha_omega.spectrum        | in air
|  Ir192_best_industries.spectrum    | in air
|  Ir192_microSelectron.spectrum     | in air
|  Ir192_beta.spectrum               | bare
|  Ir192bare.spectrum                | bare
|  Ir192_bare_1993.spectrum          | bare

 (Note that spectra from before September 1993 contained several glitches.)
