﻿#ifndef _FE_FRAMEWORK_WORLD_HPP_
#define _FE_FRAMEWORK_WORLD_HPP_
/*
Copyright © from 2022 to present, UNKNOWN STRYKER. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <FE/prerequisites.h>
#include <FE/framework/super_object_base.hpp>
#include <FE/framework/object_base.hpp>

#include <FE/pair.hxx>

#include <string>
#include <vector>




BEGIN_NAMESPACE(FE::framework)

class world final : public super_object_base
{
	FE_CLASS(world);
	FE_CLASS_HAS_A_BASE(super_object_base);

	FE_PROPERTY(m_objects_to_allocate);
	std::pmr::vector<std::pmr::string> m_objects_to_allocate;

	// Do Not Attempt to Serialize it. Raw pointers are not serializable.
	std::pmr::vector<FE::pair<FE::framework::object_base*, std::pmr::string>> m_objects_on_world;

public:
	world() noexcept;
	virtual ~world() noexcept override;

protected:
	virtual void on_construction() override;
	virtual void on_destruction() override;

	virtual void tick(FE::float64 delta_second_p) override;
};

END_NAMESPACE
#endif