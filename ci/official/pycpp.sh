#!/bin/bash
# Copyright 2023 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
source "${BASH_SOURCE%/*}/utilities/setup.sh"

#BAZEL_LLVM="C:\tools\LLVM"
#BAZEL_SH="C:\tools\msys64\usr\bin\bash.exe"
#BAZEL_VS="C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
#MSYS_NO_PATHCONV=1
#PYTHON_VERSION=3.12.3
#BAZEL_VC_FULL_VERSION=14.29.30133

#export BAZEL_LLVM=
#export BAZEL_SH=
#export BAZEL_VS=
#export MSYS_NO_PATHCONV=
#export PYTHON_VERSION=

if [[ `uname -s | grep -P '^MSYS_NT'` ]]; then
  PROFILE_JSON_PATH=$(replace_drive_letter_with_c "$TFCI_OUTPUT_DIR")
  PROFILE_JSON_PATH="$PROFILE_JSON_PATH/profile.json.gz"
else
  PROFILE_JSON_PATH="$TFCI_OUTPUT_DIR/profile.json.gz"
fi

if [[ $TFCI_PYCPP_SWAP_TO_BUILD_ENABLE == 1 ]]; then
   tfrun bazel build $TFCI_BAZEL_COMMON_ARGS --profile "$PROFILE_JSON_PATH" --config="${TFCI_BAZEL_TARGET_SELECTING_CONFIG_PREFIX}_pycpp_test"
else
   docker exec tf env
   tfrun bazel test $TFCI_BAZEL_COMMON_ARGS --profile "$PROFILE_JSON_PATH"  --config="${TFCI_BAZEL_TARGET_SELECTING_CONFIG_PREFIX}_pycpp_test"
fi

# Note: the profile can be viewed by visiting chrome://tracing in a Chrome browser.
# See https://docs.bazel.build/versions/main/skylark/performance.html#performance-profiling
tfrun bazel analyze-profile "$PROFILE_JSON_PATH"
