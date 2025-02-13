#pragma BLENDER_REQUIRE(gpu_shader_common_color_utils.glsl)
#pragma BLENDER_REQUIRE(gpu_shader_compositor_texture_utilities.glsl)

void main()
{
  ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

  vec4 color_ycca;
  rgba_to_ycca_itu_709(texture_load(input_tx, texel), color_ycca);

  color_ycca.yz = texture_load(new_chroma_tx, texel).yz;

  vec4 color_rgba;
  ycca_to_rgba_itu_709(color_ycca, color_rgba);

  imageStore(output_img, texel, color_rgba);
}
