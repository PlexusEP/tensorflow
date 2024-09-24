/* Copyright 2024 The OpenXLA Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <memory>
#include <utility>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/OwningOpRef.h"
#include "xla/python/ifrt/ir/ifrt_ir_program.h"
#include "xla/python/ifrt/serdes.h"
#include "xla/python/ifrt/support/module_parsing.h"
#include "tsl/platform/status_matchers.h"
#include "tsl/platform/statusor.h"

namespace xla {
namespace ifrt {
namespace {

using ::testing::HasSubstr;
using ::testing::Not;
using ::tsl::testing::StatusIs;

TEST(IfrtIRProgramSerDesTest, RoundTrip) {
  static constexpr absl::string_view kMlirModuleStr = R"(
!array = !ifrt.array<tensor<2xi32>, #ifrt.sharding_param<1 to [0] on 1>, [0]>
module {
  func.func @main(%arg0: !array) -> !array attributes {ifrt.function} {
    %0, %ctrl_0 = ifrt.Call @add_one::@main(%arg0) on devices [0]
        : (!array) -> !array
    return %0 : !array
  }

  module @add_one {
    func.func @main(%arg0: tensor<2xi32>) -> tensor<2xi32> {
      %0 = mhlo.constant dense<1> : tensor<2xi32>
      %1 = mhlo.add %arg0, %0 : tensor<2xi32>
      return %1 : tensor<2xi32>
    }
  }
}
  )";

  Serialized serialized;
  {
    auto context = std::make_unique<mlir::MLIRContext>();
    TF_ASSERT_OK_AND_ASSIGN(
        mlir::OwningOpRef<mlir::ModuleOp> module,
        support::ParseMlirModuleString(kMlirModuleStr, *context));
    auto program =
        std::make_unique<IfrtIRProgram>(std::move(context), std::move(module));
    TF_ASSERT_OK_AND_ASSIGN(serialized, Serialize(*program));
  }

  TF_ASSERT_OK_AND_ASSIGN(
      std::unique_ptr<IfrtIRProgram> program,
      Deserialize<IfrtIRProgram>(serialized, /*options=*/nullptr));
}

TEST(IfrtIRProgramSerDesTest, DeserializationError) {
  static constexpr absl::string_view kMlirModuleStr = R"(
!array = !ifrt.array<tensor<2xi32>, #ifrt.sharding_param<1 to [0] on 1>, [0]>
module {
  func.func @main(%arg0: !array) -> !array attributes {ifrt.function} {
    %0, %ctrl_0 = ifrt.Call @add_one::@main(%arg0) on devices [0]
        : (!array) -> !array
    return %0 : !array
  }

  module @add_one {
    func.func @main(%arg0: tensor<2xi32>) -> tensor<2xi32> {
      %0 = mhlo.constant dense<1> : tensor<2xi32>
      %1 = mhlo.add %arg0, %0 : tensor<2xi32>
      return %1 : tensor<2xi32>
    }
  }
}
  )";
  Serialized serialized;
  {
    auto context = std::make_unique<mlir::MLIRContext>();
    TF_ASSERT_OK_AND_ASSIGN(
        mlir::OwningOpRef<mlir::ModuleOp> module,
        support::ParseMlirModuleString(kMlirModuleStr, *context));
    auto program =
        std::make_unique<IfrtIRProgram>(std::move(context), std::move(module));
    TF_ASSERT_OK_AND_ASSIGN(serialized, Serialize(*program));
  }

  serialized.set_data("invalid data");

  EXPECT_THAT(Deserialize<IfrtIRProgram>(serialized, /*options=*/nullptr),
              StatusIs(Not(absl::StatusCode::kOk),
                       HasSubstr("Failed to parse IFRT IR module string")));
}

}  // namespace
}  // namespace ifrt
}  // namespace xla
