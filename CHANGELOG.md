[keep a changelog](https://keepachangelog.com/)

## Unreleased

## 0.5.0 - 2022-11-18

### Added

* DRIFT-284: Setup Doxygen and publish on readthedocs, [PR-34](https://github.com/panda-official/WaveletBuffer/pull/34/)
* DRIFT-518: Linter and cppcheck for Python bindings, [PR-36](https://github.com/panda-official/WaveletBuffer/pull/36)
* DRIFT-534: Dependencies compatibility table, [PR-37](https://github.com/panda-official/WaveletBuffer/pull/37)
* DRIFT-550: Add wrappers for Python bindings with
  docstrings, [PR-38](https://github.com/panda-official/WaveletBuffer/pull/38)
* DRIFT-579: Publish Python API references on RTD, [PR-39](https://github.com/panda-official/WaveletBuffer/pull/39)
* DRIFT-585: WaveletBuffer explanation, [PR-41](https://github.com/panda-official/WaveletBuffer/pull/41)

### Changed

* Use MacOs-12 as build system in CI, [PR-33](https://github.com/panda-official/WaveletBuffer/pull/33)
* Update conan to 1.53, [PR-40](https://github.com/panda-official/WaveletBuffer/pull/40)
* ISSUE-42: Update dependencies license page, [PR-43](https://github.com/panda-official/WaveletBuffer/pull/43)
* DRIFT-612: Fix broken pipeline for MacOS, [PR-45](https://github.com/panda-official/WaveletBuffer/pull/45)

## 0.4.0 - 2022-09-06

### Added

* Dependencies to CI, [PR-30](https://github.com/panda-official/WaveletBuffer/pull/30)

### Changed

* DRIFT-524: Replace boost by CImg, [PR-32](https://github.com/panda-official/WaveletBuffer/pull/32)

### Fixed

* DRIFT-524: Fix benchmark fixtures, [PR-32](https://github.com/panda-official/WaveletBuffer/pull/32)
* Build boost/1.73 package, [PR-30](https://github.com/panda-official/WaveletBuffer/pull/30)

## 0.3.0 - 2022-08-15

### Added

* DRIFT-507: Windows support, [PR-25](https://github.com/panda-official/WaveletBuffer/pull/25)
* DRIFT-533: MacOS support, [PR-28](https://github.com/panda-official/WaveletBuffer/pull/28)

### Changed

* DRIFT-507: Conan generator to `CMakeDeps`, [PR-25](https://github.com/panda-official/WaveletBuffer/pull/25)

## 0.2.0 - 2022-07-22

### Added

* DRIFT-493: WaveletImage, [PR-21](https://github.com/panda-official/WaveletBuffer/pull/21)
* Required version for conan, [PR-27](https://github.com/panda-official/WaveletBuffer/pull/27)

### Changed

* DRIFT-506: Upload python packages to PyPI server, [PR-23](https://github.com/panda-official/WaveletBuffer/pull/23)

### Removed

* DRIFT-512: WaveletParameters.is_raw_convolve_1d, [PR-24](https://github.com/panda-official/WaveletBuffer/pull/24)

## 0.1.1 - 2022-07-12

### Fixed

* Fix sdist Python package, [PR-20](https://github.com/panda-official/WaveletBuffer/pull/20)
* DRIFT-507: Bugs detected when adding Windows support, [PR-25](https://github.com/panda-official/WaveletBuffer/pull/25)

## 0.1.0 - 2022-07-07

### Added

* DRIFT-445: Conan integration, [PR-9](https://github.com/panda-official/WaveletBuffer/pull/9)
* DRIFT-446: New python bindings, [PR-12](https://github.com/panda-official/WaveletBuffer/pull/12)
* DRIFT-447: MacOS instructions, [PR-14](https://github.com/panda-official/WaveletBuffer/pull/14)
* DRIFT-448: Code coverage support, [PR-10](https://github.com/panda-official/WaveletBuffer/pull/10)
* DRIFT-480: More unit tests, [PR-11](https://github.com/panda-official/WaveletBuffer/pull/11)
* Installing section in README, [PR-13](https://github.com/panda-official/WaveletBuffer/pull/13)

### Changed

* DRIFT-480: Bump up Catch version to 3.0.1, [PR-11](https://github.com/panda-official/WaveletBuffer/pull/11)
* DRIFT-491: Move needed metric to wavelet buffer, remove metric with
  dependencies, [PR-15](https://github.com/panda-official/WaveletBuffer/pull/15/checks)

### Fixed

* DRIFT-480: Set `BLAZE_USE_SHARED_MEMORY_PARALLELIZATION=0` to avoid `Nested parallel sections detected` blaze
  problem, [PR-11](https://github.com/panda-official/WaveletBuffer/pull/11)
* DRIFT-500: 2d Amplitude bug, [PR-17](https://github.com/panda-official/WaveletBuffer/pull/17)
