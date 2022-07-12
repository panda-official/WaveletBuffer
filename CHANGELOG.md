## Release 0.1.1 (2022-07-12)

### Fixed

* Fix sdist Python package, [PR-20](https://github.com/panda-official/WaveletBuffer/pull/20)

## Release 0.1.0 (2022-07-07)

### Added

* DRIFT-445: Conan integration, [PR-9](https://github.com/panda-official/WaveletBuffer/pull/9)
* DRIFT-446: New python bindings, [PR-12](https://github.com/panda-official/WaveletBuffer/pull/12)
* DRIFT-447: MacOS instructions, [PR-14](https://github.com/panda-official/WaveletBuffer/pull/14)
* DRIFT-448: Code coverage support, [PR-10](https://github.com/panda-official/WaveletBuffer/pull/10)
* DRIFT-480: More unit tests, [PR-11](https://github.com/panda-official/WaveletBuffer/pull/11)
* Installing section in README, [PR-13](https://github.com/panda-official/WaveletBuffer/pull/13)

### Changed

* DRIFT-480: Bump up Catch version to 3.0.1, [PR-11](https://github.com/panda-official/WaveletBuffer/pull/11)
* DRIFT-491: Move needed metric to wavelet buffer, remove metric with dependencies, [PR-15](https://github.com/panda-official/WaveletBuffer/pull/15/checks)

### Fixed

* DRIFT-480: Set `BLAZE_USE_SHARED_MEMORY_PARALLELIZATION=0` to avoid `Nested parallel sections detected` blaze
  problem, [PR-11](https://github.com/panda-official/WaveletBuffer/pull/11)
* DRIFT-500: 2d Amplitude bug, [PR-17](https://github.com/panda-official/WaveletBuffer/pull/17)