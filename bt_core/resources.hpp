#pragma once

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include <vector>

namespace behaviour_tree {
class IBehaviourTreeNodeBehaviour;

void WriteStringToFile(Ref<FileAccess> &file, const String &key);
void WriteVariantToFile(Ref<FileAccess> &file, const Variant &var);
String ReadStringFromFile(Ref<FileAccess> &file);
Variant ReadVariantFromFile(Ref<FileAccess> &file);


class ResourceFormatLoaderBehaviourTree : public ResourceFormatLoader {
public:
	Ref<Resource> load(
			const String &p_path,
			const String &p_original_path = "",
			Error *r_error = nullptr,
			bool p_use_sub_threads = false,
			float *r_progress = nullptr,
			CacheMode p_cache_mode = CACHE_MODE_REUSE) override;
	void get_recognized_extensions(List<String> *r_extensions) const override;
	bool handles_type(const String &p_type) const override;
	String get_resource_type(const String &p_path) const override;
};

class ResourceFormatSaverBehaviourTree : public ResourceFormatSaver {
public:
	Error save(const Ref<Resource> &p_resource, const String &p_path, uint32_t p_flags = 0) override;
	bool recognize(const Ref<Resource> &p_resource) const override;
	void get_recognized_extensions(const Ref<Resource> &p_resource, List<String> *r_extensions) const override;
};
} //namespace behaviour_tree
