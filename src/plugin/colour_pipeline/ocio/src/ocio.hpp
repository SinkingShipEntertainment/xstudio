// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cmath>
#include <cfloat>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <typeinfo>

#include <OpenColorIO/OpenColorIO.h> //NOLINT

#include "xstudio/colour_pipeline/colour_pipeline_actor.hpp"
#include "xstudio/plugin_manager/plugin_manager.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/global_store/global_store.hpp"
#include "xstudio/thumbnail/thumbnail.hpp"
#include "ui_text.hpp"

namespace OCIO = OCIO_NAMESPACE;


namespace xstudio::colour_pipeline {

class OCIOColourPipeline : public ColourPipeline {
  private:
    struct PerConfigSettings {
        std::string display;
        std::string popout_viewer_display;
        std::string view;
    };

    struct MediaParams {
        utility::Uuid source_uuid;

        OCIO::ConstConfigRcPtr ocio_config;
        std::string ocio_config_name;

        utility::JsonStore metadata;
        std::string user_input_cs;
        std::string user_input_display;
        std::string user_input_view;

        OCIO::GradingPrimary primary = OCIO::GradingPrimary(OCIO::GRADING_LIN);

        std::string compute_hash() const;
    };

    struct ShaderDescriptors {
        OCIO::ConstGpuShaderDescRcPtr main_viewer_shader_desc;
        OCIO::ConstGpuShaderDescRcPtr popout_viewer_shader_desc;
        std::mutex mutex;
    };
    typedef std::shared_ptr<ShaderDescriptors> ShaderDescriptorsPtr;

  public:
    explicit OCIOColourPipeline(const utility::JsonStore &s);

    void register_hotkeys() override;

    static std::string name();
    const utility::Uuid &class_uuid() const override;
    ColourPipelineDataPtr make_empty_data() const override;

    // Generates a hash of the shader for the whole display pipeline,
    // this is used by xStudio to implement caching mechanisms.
    std::string compute_hash(
        const utility::Uuid &source_uuid, const utility::JsonStore &colour_params) override;

    // Construct the display pipeline shader, including the code
    // and resources (eg. textures for LUTs) as needed.
    void setup_shader(
        ColourPipelineData &pipe_data,
        const utility::Uuid &source_uuid,
        const utility::JsonStore &colour_params) override;

    // Update colour pipeline shader dynamic parameters.
    void update_shader_uniforms(
        ColourPipelineDataPtr &pipe_data, const utility::Uuid &source_uuid) override;

    thumbnail::ThumbnailBufferPtr process_thumbnail(
        const media::AVFrameID &media_ptr, const thumbnail::ThumbnailBufferPtr &buf) override;

    std::string fast_display_transform_hash(const media::AVFrameID &media_ptr) override;

    // GUI handling
    void media_source_changed(
        const utility::Uuid &source_uuid, const utility::JsonStore &colour_params) override;
    void attribute_changed(const utility::Uuid &attribute_uuid, const int /*role*/) override;
    void hotkey_pressed(const utility::Uuid &hotkey_uuid, const std::string &context) override;
    void hotkey_released(const utility::Uuid &hotkey_uuid, const std::string &context) override;
    bool pointer_event(const ui::PointerEvent &e) override;
    void screen_changed(
        const bool &is_primary_viewer,
        const std::string &name,
        const std::string &model,
        const std::string &manufacturer,
        const std::string &serialNumber) override;

  private:
    MediaParams get_media_params(
        const utility::Uuid &source_uuid,
        const utility::JsonStore &colour_params = utility::JsonStore()) const;

    void
    set_media_params(const utility::Uuid &source_uuid, const MediaParams &media_param) const;

    // OCIO Transform helpers

    const char *working_space(const MediaParams &media_param) const;

    OCIO::TransformRcPtr source_transform(const MediaParams &media_param) const;

    const char *
    default_display(const MediaParams &media_param, const std::string &monitor_name = "") const;

    OCIO::TransformRcPtr display_transform(
        const std::string &source,
        const std::string &display,
        const std::string &view,
        OCIO::TransformDirection direction) const;

    OCIO::TransformRcPtr identity_transform() const;

    // OCIO setup

    OCIO::ConstConfigRcPtr load_ocio_config(const std::string &config_name) const;

    OCIO::ConstProcessorRcPtr make_processor(
        const MediaParams &media_param, bool is_main_viewer, bool is_thumbnail) const;

    OCIO::ConstProcessorRcPtr make_dynamic_display_processor(
        const MediaParams &media_param,
        const OCIO::ConstConfigRcPtr &config,
        const OCIO::ConstContextRcPtr &context,
        const OCIO::GroupTransformRcPtr &group,
        const std::string &display,
        const std::string &view,
        const std::string &look_name,
        const std::string &cdl_file_name) const;

    OCIO::ConstGpuShaderDescRcPtr
    make_shader(OCIO::ConstProcessorRcPtr &processor, bool is_main_viewer) const;

    void setup_textures(
        OCIO::ConstGpuShaderDescRcPtr &shader_desc,
        ColourPipelineData &data,
        bool is_main_viewer) const;

    // OCIO dynamic properties

    void update_dynamic_parameters(
        OCIO::ConstGpuShaderDescRcPtr &shader, const utility::Uuid &source_uuid) const;

    void update_all_uniforms(
        OCIO::ConstGpuShaderDescRcPtr &shader,
        ColourPipelineDataPtr &data,
        const utility::Uuid &source_uuid) const;

    // GUI handling

    void setup_ui();

    void populate_ui(const MediaParams &media_param);

    std::vector<std::string> parse_display_views(
        OCIO::ConstConfigRcPtr ocio_config,
        std::map<std::string, std::vector<std::string>> &view_map) const;

    std::vector<std::string> parse_all_colourspaces(OCIO::ConstConfigRcPtr ocio_config) const;

    void update_views(OCIO::ConstConfigRcPtr ocio_config);

    void update_bypass(module::StringChoiceAttribute *viewer, bool bypass);

  private:
    // We currently use global mutexes for simplicity, this means any
    // lookup in those maps will be serialized. A better strategy would
    // be a double lock approach where each items have their own lock
    // so that multiple request on different items can be truly parallel.
    // The above approach is used in OCIO (eg. look for "g_fileCache").
    // Waiting for performance feedback first before optimising further...
    mutable std::mutex pipeline_cache_mutex_;
    mutable std::map<std::string, std::string> pipeline_cache_;
    mutable std::mutex ocio_config_cache_mutex_;
    mutable std::map<std::string, OCIO::ConstConfigRcPtr> ocio_config_cache_;
    mutable std::mutex media_params_mutex_;
    mutable std::map<utility::Uuid, MediaParams> media_params_;
    mutable std::mutex per_config_settings_mutex_;
    mutable std::map<std::string, PerConfigSettings> per_config_settings_;

    // GUI handling
    bool ui_initialized_ = false;
    UiText ui_text_;

    module::StringChoiceAttribute *channel_;
    module::StringChoiceAttribute *display_;
    module::StringChoiceAttribute *popout_viewer_display_;
    module::StringChoiceAttribute *view_;
    module::FloatAttribute *exposure_;
    module::StringChoiceAttribute *source_colour_space_;
    module::BooleanAttribute *colour_bypass_;

    std::map<utility::Uuid, std::string> channel_hotkeys_;
    utility::Uuid exposure_hotkey_;
    utility::Uuid reset_hotkey_;

    // Holds info about the currently on screen media
    utility::Uuid current_source_uuid_;
    std::string current_config_name_;
    MediaParams current_source_media_params_;

    // Holds data on display screen option
    std::string main_monitor_name_;
    std::string popout_monitor_name_;
};

} // namespace xstudio::colour_pipeline
