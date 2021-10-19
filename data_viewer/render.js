let gl_point_buffers = {};
let gl_line_buffers = {};
let gl_area_buffers = {};
let gl_rectangle_buffers = {};
let gl_rectangle_buffer;

let gl;
let shader;
let color_shader;

function xyz_to_rgb(color) {
	return [
		color[0] * 3.2406 + color[1] * -1.5372 + color[2] * -0.4986,
		color[0] * -0.9689 + color[1] * 1.8758 + color[2] * 0.0415,
		color[0] * 0.0557 + color[1] * -0.2040 + color[2] * 1.0570
	];
}

function rgb_to_srgb(color) {
	return color.map(e => 1.055 * Math.pow(e, 1 / 2.4));
}

function xyz_to_srgb8(color) {
	return rgb_to_srgb(xyz_to_rgb(color)).map(e => Math.floor(e * 255));
}

function amplify_density_values(values, exponent) {
	values = values.map(e => Math.pow(e, exponent));
	let sum = values.reduce((acc, e) => acc + e, 0);
	return values.map(e => e / sum);
}

function drawLineGraph(chunks, model_view, proj, key, density_key) {

	gl.useProgram(shader.program);

	gl.uniformMatrix4fv(shader.model_view, false, model_view);
	gl.uniformMatrix4fv(shader.proj, false, proj);

	gl.uniform4fv(shader.color, [0.0, 0.0, 0.0, 1.0]);

	for (let i = 0; i < chunks.length; ++i) {
		let chunk = chunks[i];
		if (!(key in chunk.data))
			return;
		let graph_id = [chunk.id, key, density_key].join("");
		if (!(graph_id in gl_line_buffers))
			gl_line_buffers[graph_id] = chunk.getLineBuffer(gl, key, density_key);
		gl.bindBuffer(gl.ARRAY_BUFFER, gl_line_buffers[graph_id].buffer);
		gl.vertexAttribPointer(
			shader.position,
			2,
			gl.FLOAT,
			false,
			0,
			0);
		gl.enableVertexAttribArray(shader.position);

		if (density_key)
			gl.drawArrays(gl.LINES, 0, chunk.size * 2);
		else
			gl.drawArrays(gl.LINE_STRIP, 0, chunk.size);
	}
}

function drawAreaGraph(chunks, model_view, proj, color, key, density_key) {

	gl.useProgram(shader.program);

	gl.uniformMatrix4fv(shader.model_view, false, model_view);
	gl.uniformMatrix4fv(shader.proj, false, proj);

	gl.uniform4fv(shader.color, color);

	for (let i = 0; i < chunks.length; ++i) {
		let chunk = chunks[i];
		if (!(key in chunk.data))
			return;
		let graph_id = [chunk.id, key, density_key].join("");
		if (!(graph_id in gl_area_buffers))
			gl_area_buffers[graph_id] = chunk.getAreaBuffer(gl, key, density_key);
		gl.bindBuffer(gl.ARRAY_BUFFER, gl_area_buffers[graph_id].buffer);
		gl.vertexAttribPointer(
			shader.position,
			2,
			gl.FLOAT,
			false,
			0,
			0);
		gl.enableVertexAttribArray(shader.position);

		if (density_key)
			gl.drawArrays(gl.TRIANGLES, 0, chunk.size * 6);
		else
			gl.drawArrays(gl.TRIANGLE_STRIP, 0, chunk.size * 2);
	}
}

function drawPointGraph(chunks, model_view, proj, key, density_key) {

	gl.useProgram(shader.program);

	gl.uniformMatrix4fv(shader.model_view, false, model_view);
	gl.uniformMatrix4fv(shader.proj, false, proj);

	gl.uniform4fv(shader.color, [0.0, 0.0, 0.0, 1.0]);

	for (let i = 0; i < chunks.length; ++i) {
		let chunk = chunks[i];
		if (!(key in chunk.data))
			return;
		let graph_id = [chunk.id, key, density_key].join("");
		if (!(graph_id in gl_point_buffers))
			gl_point_buffers[graph_id] = chunk.getPointBuffer(gl, key, density_key);
		gl.bindBuffer(gl.ARRAY_BUFFER, gl_point_buffers[graph_id].buffer);
		gl.vertexAttribPointer(
			shader.position,
			2,
			gl.FLOAT,
			false,
			0,
			0);
		gl.enableVertexAttribArray(shader.position);

		gl.drawArrays(gl.POINTS, 0, chunk.size);
	}

}

function drawRectangles(chunks, model_view, proj, colors, keys, density_key) {

	gl.useProgram(color_shader.program);

	gl.uniformMatrix4fv(color_shader.model_view, false, model_view);
	gl.uniformMatrix4fv(color_shader.proj, false, proj);

	gl.uniformMatrix4fv(color_shader.color_modifier, false, colors.flat());

	for (let i = 0; i < chunks.length; ++i) {
		let chunk = chunks[i];
		if (keys.some(key => !key in chunk.data))
			return;
		let graph_id = [chunk.id, keys, density_key].join("");
		if (!(graph_id in gl_rectangle_buffers))
			gl_rectangle_buffers[graph_id] = chunk.getRectangleBuffer(gl, keys, density_key);
		let buffer = gl_rectangle_buffers[graph_id];
		gl.bindBuffer(gl.ARRAY_BUFFER, buffer.buffer);
		gl.vertexAttribPointer(
			color_shader.position,
			2,
			gl.FLOAT,
			false,
			0,
			buffer.offset.position);
		gl.enableVertexAttribArray(color_shader.position);
		gl.vertexAttribPointer(
			color_shader.color,
			4,
			gl.UNSIGNED_BYTE,
			true,
			0,
			buffer.offset.color);
		gl.enableVertexAttribArray(color_shader.color);

		gl.drawArrays(gl.TRIANGLES, 0, buffer.vertex_count);
	}
}

function drawRectangle(clip, x, y, w, h, color) {

	if (gl_rectangle_buffer == null)
		gl_rectangle_buffer = createBuffer(gl, new Float32Array([
			0.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,
			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0
		]));
	
	gl.useProgram(shader.program);

	const model_view = [
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	];

	const proj = [
		2.0 * w / clip.rect.w, 0.0, 0.0, 0.0,
		0.0, 2.0 * h / clip.rect.h, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		2.0 * x / clip.rect.w - 1.0, 2.0 * y / clip.rect.h - 1.0, 0.0, 1.0
	];

	gl.uniformMatrix4fv(shader.model_view, false, model_view);
	gl.uniformMatrix4fv(shader.proj, false, proj);

	gl.uniform4fv(shader.color, color);

	gl.bindBuffer(gl.ARRAY_BUFFER, gl_rectangle_buffer.buffer);
	gl.vertexAttribPointer(
		shader.position,
		2,
		gl.FLOAT,
		false,
		0,
		0);
	gl.enableVertexAttribArray(shader.position);

	gl.drawArrays(gl.TRIANGLES, 0, 6);

}

function drawZoomArea(clip) {
	if (zoom_area_start) {
		let zoom_area_screen_start = (zoom_area_start - focus) / screen_to_real + clip.rect.w * 0.5;
		let zoom_area_screen_end = view_mouse_x;
		let zoom_area_screen_left = Math.min(zoom_area_screen_start, zoom_area_screen_end);
		let zoom_area_screen_right = Math.max(zoom_area_screen_start, zoom_area_screen_end);
		let zoom_area_screen_width = zoom_area_screen_right - zoom_area_screen_left;
		let color = [0.4, 0.4, 0.8, 0.5];
		if (zoom_area_screen_width < min_zoom_area_width)
			color = [0.2, 0.2, 0.4, 0.25];
		drawRectangle(clip, zoom_area_screen_left, 0, zoom_area_screen_width, clip.rect.h, color);
	}
}

function getVisibleChunks(minimum_bin_size, left, right) {

	let bin_size = base_bin_size;
	for (let i = 0; i < bin_sizes.length; ++i) {
		if (bin_sizes[i] <= minimum_bin_size)
		bin_size = bin_sizes[i];
		else
			break;
	}

	let max_chunk_index = Math.floor(chr.size / (chunk_size * bin_size));

	let visible_chunks = [];
	let first_index = Math.floor((focus - document.body.clientWidth * 0.5 * screen_to_real - chr.offset) / (chunk_size * bin_size));
	first_index = Math.max(0, first_index);
	let last_index = Math.floor((focus + document.body.clientWidth * 0.5 * screen_to_real - chr.offset) / (chunk_size * bin_size));
	last_index = Math.min(max_chunk_index, last_index);
	for (let i = first_index; i <= last_index; ++i) {
		visible_chunks.push(get_chunk(chr.name, bin_size, i));
	}

	/* if (first_index > 0) {
		let look_ahead_index = first_index - 1;
		get_chunk(chr.name, bin_size, look_ahead_index);
	}
	if (last_index < max_chunk_index) {
		let look_ahead_index = last_index + 1;
		get_chunk(chr.name, bin_size, look_ahead_index);
	} */

	if (visible_chunks.some(e => e === null)) {
		if (bin_size < max_bin_size)
			return getVisibleChunks(bin_size * 2, left, right);
		return [null, null];
	}

	return [visible_chunks, bin_size];
}

function initContext() {

	gl = canvas.getContext("webgl");

	const vert_source = `
		attribute vec4 position;
		uniform mat4 model_view;
		uniform mat4 proj;

		void main() {
			gl_Position = proj * model_view * position;
			gl_PointSize = 1.0;
		}
	`;

	const frag_source = `
		uniform lowp vec4 color;

		void main() {
			gl_FragColor = color;
		}
	`;

	const color_vert_source = `
		uniform mat4 model_view;
		uniform mat4 proj;
		uniform mat4 color_modifier;

		attribute vec4 position;
		attribute vec4 color;

		varying lowp vec4 v_color;

		void main() {
			gl_Position = proj * model_view * position;
			gl_PointSize = 1.0;
			v_color = color_modifier * color;
		}
	`;

	const color_frag_source = `
		varying lowp vec4 v_color;

		void main() {
			gl_FragColor = v_color;
		}
	`;
	
	if (shader == null)
		shader = new Shader(gl, vert_source, frag_source, [
			"position"
		], [
			"model_view",
			"proj",
			"color"
		]);
	if (color_shader == null)
		color_shader = new Shader(gl, color_vert_source, color_frag_source, [
			"position",
			"color"
		], [
			"model_view",
			"proj",
			"color_modifier"
		]);
}

function render(repaint) {

	initContext();

	let rect = document.documentElement.getBoundingClientRect();
	canvas_wrapper.style.width = rect.width + "px";
	canvas_wrapper.style.height = rect.height + "px";
	if (canvas.width < rect.width || 
		canvas.height < rect.height) {
		repaint = true;
		canvas.width = rect.width;
		canvas.height = rect.height;
	}

	if (repaint) {
		console.log("REPAINT");
	}

	// if (false)
	{
		let overview_clips = clips.filter(e => e.type == "overview");
		for (let clip of overview_clips) {
			if (clip.subclips) {
				for (let i = 0; i < clip.subclips.length; ++i) {
					let subclip = clip.subclips[i];
					let data = clip.data[0];

					let repaint_clip = repaint;

					let rect = subclip.render_area.getBoundingClientRect();
					rect.y += window.scrollY;

					if (subclip.rect.x != rect.x || 
						subclip.rect.y != rect.y || 
						subclip.rect.w != rect.width || 
						subclip.rect.h != rect.height) {
						repaint_clip = true;
						subclip.rect.x = rect.x;
						subclip.rect.y = rect.y;
						subclip.rect.w = rect.width;
						subclip.rect.h = rect.height;
					}

					if (subclip.render_out_of_date) {
						repaint_clip = true;
						subclip.render_out_of_date = false;
					}		

					if (repaint_clip === false) {
						continue;
					}

					gl.enable(gl.SCISSOR_TEST);
	
					gl.viewport(subclip.rect.x, canvas.height - subclip.rect.y - subclip.rect.h, subclip.rect.w, subclip.rect.h);
					gl.scissor(subclip.rect.x, canvas.height - subclip.rect.y - subclip.rect.h, subclip.rect.w, subclip.rect.h);
					gl.clearColor(1.0, 1.0, 1.0, 1.0);
					gl.clear(gl.COLOR_BUFFER_BIT);
	
					gl.enable(gl.BLEND);
					gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

					max_chunk_index = Math.floor(chromosome_sizes[i] / (chunk_size * max_bin_size));
					let chunks = [];
					for (let j = 0; j <= max_chunk_index; ++j) {
						chunks.push(get_chunk(chromosome_names[i], max_bin_size, j));
					}
					
					if (chunks.some(e => e === null))
						continue;

					const model_view = [
						1.0, 0.0, 0.0, 0.0,
						0.0, 1.0, 0.0, 0.0,
						0.0, 0.0, 1.0, 0.0,
						-chromosome_sizes[i] * 0.5, 0.0, 0.0, 1.0
					];
		
					const proj = [
						2.0 / chromosome_sizes[i] / initial_view_margin, 0.0, 0.0, 0.0,
						0.0, 2.0, 0.0, 0.0,
						0.0, 0.0, 1.0, 0.0,
						0.0, -1.0, 0.0, 1.0
					];
		
					drawRectangles(chunks, model_view, proj, data.colors, data.keys, data.density_key);
				}
			}
			if (clip.render_area) {

				let current_chromosome_overview_a = overview_clips.find(e => e.subclips).subclips[chromosome_index].render_area.parentNode;
				let zoom_overlay = current_chromosome_overview_a.querySelector(".overview_zoom_overlay");
				let zoom_rect = zoom_overlay.getBoundingClientRect();
				
				let repaint_clip = repaint;

				let rect = clip.render_area.getBoundingClientRect();
				rect.y += window.scrollY;

				if (clip.rect.x != rect.x || 
					clip.rect.y != rect.y || 
					clip.rect.w != rect.width || 
					clip.rect.h != rect.height) {
					repaint_clip = true;
					clip.rect.x = rect.x;
					clip.rect.y = rect.y;
					clip.rect.w = rect.width;
					clip.rect.h = rect.height;
				}

				if (clip.render_out_of_date) {
					repaint_clip = true;
					clip.render_out_of_date = false;
				}

				if (repaint_clip === false) {
					continue;
				}

				gl.enable(gl.SCISSOR_TEST);

				gl.viewport(clip.rect.x, canvas.height - clip.rect.y - clip.rect.h, clip.rect.w, clip.rect.h);
				gl.scissor(clip.rect.x, canvas.height - clip.rect.y - clip.rect.h, clip.rect.w, clip.rect.h);
				gl.clearColor(1.0, 1.0, 1.0, 1.0);
				gl.clear(gl.COLOR_BUFFER_BIT);

				gl.enable(gl.BLEND);
				gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

				const line_width_left = 2.0 / Math.cos(Math.PI / 2.0 - Math.atan(rect.height / zoom_rect.left));
				const line_width_right = 2.0 / Math.cos(Math.PI / 2.0 - Math.atan(rect.height / (rect.right - zoom_rect.right)));
				let buffer_data = new Float32Array([
					-line_width_left,
					0.0,
					0.0,
					0.0,
					zoom_rect.left - line_width_left,
					rect.height,
					0.0,
					0.0,
					zoom_rect.left,
					rect.height,
					zoom_rect.left - line_width_left,
					rect.height,
					rect.right,
					0.0,
					rect.right + line_width_right,
					0.0,
					zoom_rect.right,
					rect.height,
					rect.right + line_width_right,
					0.0,
					zoom_rect.right + line_width_right,
					rect.height,
					zoom_rect.right,
					rect.height
				]);

				if (clip.gl_zoom_lines_buffer == null)
					clip.gl_zoom_lines_buffer = createDynamicBuffer(gl, buffer_data);
				else
					updateDynamicBuffer(gl, clip.gl_zoom_lines_buffer, buffer_data);
	
				gl.useProgram(shader.program);
			
				const model_view = [
					1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 0.0, 0.0, 1.0
				];
			
				const proj = [
					2.0 / clip.rect.w, 0.0, 0.0, 0.0,
					0.0, 2.0 / clip.rect.h, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					-1.0, -1.0, 0.0, 1.0
				];
			
				gl.uniformMatrix4fv(shader.model_view, false, model_view);
				gl.uniformMatrix4fv(shader.proj, false, proj);
			
				gl.uniform4fv(shader.color, [0, 0, 0, 1]);
			
				gl.bindBuffer(gl.ARRAY_BUFFER, clip.gl_zoom_lines_buffer.buffer);
				gl.vertexAttribPointer(
					shader.position,
					2,
					gl.FLOAT,
					false,
					0,
					0);
				gl.enableVertexAttribArray(shader.position);
			
				gl.drawArrays(gl.TRIANGLES, 0, 12);
				
				// let ctx = canvas.getContext("2d");

				// ctx.setTransform(1, 0, 0, 1, 0, 0);
				// ctx.clearRect(0, 0, canvas.width, canvas.height);

				// if (zoom_area_start) {
				// 	let zoom_area_screen_start = (zoom_area_start - focus) / screen_to_real + canvas.width * 0.5;
				// 	let zoom_area_screen_end = view_mouse_x;
				// 	let zoom_area_screen_left = Math.min(zoom_area_screen_start, zoom_area_screen_end);
				// 	let zoom_area_screen_right = Math.max(zoom_area_screen_start, zoom_area_screen_end);
				// 	let zoom_area_screen_width = zoom_area_screen_right - zoom_area_screen_left;
				// 	ctx.fillStyle = "rgba(100, 100, 240, 0.5)";
				// 	if (zoom_area_screen_width < min_zoom_area_width)
				// 		ctx.fillStyle = "rgba(100, 100, 240, 0.25)";
				// 	let zoom_area_overview_left = rect.left + zoom_area_screen_left / canvas.width * rect.width;
				// 	let zoom_area_overview_right = rect.left + zoom_area_screen_right / canvas.width * rect.width;

				// 	ctx.beginPath();
				// 	ctx.moveTo(zoom_area_screen_left, canvas.height);
				// 	ctx.lineTo(zoom_area_overview_left, 0);
				// 	ctx.lineTo(zoom_area_overview_right, 0);
				// 	ctx.lineTo(zoom_area_screen_right, canvas.height);
				// 	ctx.fill();
				// }

				// ctx.lineWidth = 2;
				// ctx.beginPath();
				// ctx.moveTo(0, canvas.height);
				// ctx.lineTo(rect.left, 0);
				// ctx.moveTo(canvas.width, canvas.height);
				// ctx.lineTo(rect.right, 0);
				// ctx.stroke();
			}
		}
	}

	let half_size = screen_to_real * document.body.clientWidth * 0.5;
	let [visible_chunks, bin_size] = getVisibleChunks(screen_to_real * resolution_modifier, focus - half_size, focus + half_size);

	let annotation_clips = clips.filter(e => e.type == "annotation");
	for (const clip of annotation_clips) {

		let repaint_clip = repaint;

		let rect = clip.render_area.getBoundingClientRect();
		rect.y += window.scrollY;

		if (clip.rect.x != rect.x || 
			clip.rect.y != rect.y || 
			clip.rect.w != rect.width || 
			clip.rect.h != rect.height) {
			repaint_clip = true;
			clip.rect.x = rect.x;
			clip.rect.y = rect.y;
			clip.rect.w = rect.width;
			clip.rect.h = rect.height;
		}

		if (clip.render_out_of_date) {
			repaint_clip = true;
			clip.render_out_of_date = false;
		}

		if (repaint_clip === false) {
			continue;
		}

		gl.enable(gl.SCISSOR_TEST);

		gl.viewport(clip.rect.x, canvas.height - clip.rect.y - clip.rect.h, clip.rect.w, clip.rect.h);
		gl.scissor(clip.rect.x, canvas.height - clip.rect.y - clip.rect.h, clip.rect.w, clip.rect.h);
		gl.clearColor(1.0, 1.0, 1.0, 1.0);
		gl.clear(gl.COLOR_BUFFER_BIT);

		gl.enable(gl.BLEND);
		gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);
		
		if (chr.name in clip.chromosomes) {
			let annotations = clip.chromosomes[chr.name];
			if (annotations) {

				if (annotations.gl_buffer == null)
					annotations.gl_buffer = createBuffer(gl, annotations.gl_buffer_data);

				gl.bindBuffer(gl.ARRAY_BUFFER, annotations.gl_buffer.buffer);
				gl.vertexAttribPointer(
					shader.position,
					2,
					gl.FLOAT,
					false,
					0,
					0);
				gl.enableVertexAttribArray(shader.position);

				gl.useProgram(shader.program);

				const model_view = [
					1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					-focus, 0.0, 0.0, 1.0
				];

				const proj = [
					2.0 / clip.rect.w / screen_to_real, 0.0, 0.0, 0.0,
					0.0, -2.0 * 10.0 / clip.rect.h, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 1.0, 0.0, 1.0
				];

				gl.uniformMatrix4fv(shader.model_view, false, model_view);
				gl.uniformMatrix4fv(shader.proj, false, proj);

				gl.uniform4fv(shader.color, [0.0, 0.0, 0.0, 0.5]);

				gl.drawArrays(gl.TRIANGLES, 0, 2 * 3 * annotations.count);
			}
		} else if (clip.lookup) {
			clip.chromosomes[chr.name] = null;

			// make some copies to prevent reference capturing fuckery
			let chr_name_locked = chr.name;
			let clip_locked = clip;
			
			fetch(clip.source + chr.name + ".ann")
			.then(response => {
				if (response.ok)
					return response.arrayBuffer()
				else
					return null;
			})
			.then(arrayBuffer => {
				if (arrayBuffer === null)
					return;

				let annotation_count = new Uint32Array(arrayBuffer, 0, 1)[0];
				let offset = 8;

				let annotations = {};
				annotations.count = annotation_count;

				for (const data of clip_locked.lookup.data_sets) {
					offset += 7 - (offset - 1) % 8;
					switch (data.size) {
						case 1:
							annotations[data.name] = new Uint8Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count;
							break;
						case 2:
							annotations[data.name] = new Uint16Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 2;
							break;
						case 4:
							annotations[data.name] = new Uint32Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 4;
							break;
					}
				}

				for (const data of clip_locked.lookup.data_prefixes) {
					offset += 7 - (offset - 1) % 8;
					switch (data.size) {
						case 1:
							annotations[data.name] = new Uint8Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count;
							break;
						case 2:
							annotations[data.name] = new Uint16Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 2;
							break;
						case 4:
							annotations[data.name] = new Uint32Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 4;
							break;
					}
				}

				for (const data of clip_locked.lookup.data_values) {
					offset += 7 - (offset - 1) % 8;
					switch (data.size) {
						case 1:
							annotations[data.name] = new Uint8Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count;
							break;
						case 2:
							annotations[data.name] = new Uint16Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 2;
							break;
						case 4:
							annotations[data.name] = new Uint32Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 4;
							break;
					}
				}

				let stack = [];

				const stride = 2 * 3 * 2;
				annotations.gl_buffer_data = new Float32Array(stride * annotations.count);
				
				for (let i = 0; i < annotations.count; ++i) {

					if (stack.some(e => annotations.start[i] == annotations.start[e] && annotations.end[i] == annotations.end[e]))
						continue;

					for (let j = 0; j < stack.length; ++j) {
						if (stack[j] != null && annotations.end[stack[j]] < annotations.start[i])
							stack[j] = null;
					}

					let height_order = stack.findIndex(e => e == null);
					if (height_order == -1) {
						height_order = stack.length;
						stack.push(i);
					} else {
						stack[height_order] = i;
					}

					const margin = 0.06125;

					const top = height_order + margin;
					const bottom = height_order + 1.0 - margin;

					annotations.gl_buffer_data[i * stride] = annotations.start[i];
					annotations.gl_buffer_data[i * stride + 1] = top;
					annotations.gl_buffer_data[i * stride + 2] = annotations.start[i];
					annotations.gl_buffer_data[i * stride + 3] = bottom;
					annotations.gl_buffer_data[i * stride + 4] = annotations.end[i] + 1;
					annotations.gl_buffer_data[i * stride + 5] = top;

					annotations.gl_buffer_data[i * stride + 6] = annotations.end[i] + 1;
					annotations.gl_buffer_data[i * stride + 7] = top;
					annotations.gl_buffer_data[i * stride + 8] = annotations.end[i] + 1;
					annotations.gl_buffer_data[i * stride + 9] = bottom;
					annotations.gl_buffer_data[i * stride + 10] = annotations.start[i];
					annotations.gl_buffer_data[i * stride + 11] = bottom;
				}

				clip_locked.chromosomes[chr_name_locked] = annotations;

				// clip_locked.render_out_of_date = true;
				render_out_of_date = true;
			});
		}
	
		drawZoomArea(clip);
	}

	let chromosome_clips = clips.filter(e => e.type == null);
	if (visible_chunks) {
		for (const clip of chromosome_clips) {
	
			let repaint_clip = repaint;

			let rect = clip.render_area.getBoundingClientRect();
			rect.y += window.scrollY;
	
			if (clip.rect.x != rect.x || 
				clip.rect.y != rect.y || 
				clip.rect.w != rect.width || 
				clip.rect.h != rect.height) {
				repaint_clip = true;
				clip.rect.x = rect.x;
				clip.rect.y = rect.y;
				clip.rect.w = rect.width;
				clip.rect.h = rect.height;
			}
	
			if (clip.render_out_of_date) {
				repaint_clip = true;
				clip.render_out_of_date = false;
			}
	
			if (repaint_clip === false) {
				continue;
			}
	
			gl.enable(gl.SCISSOR_TEST);

			gl.viewport(clip.rect.x, canvas.height - clip.rect.y - clip.rect.h, clip.rect.w, clip.rect.h);
			gl.scissor(clip.rect.x, canvas.height - clip.rect.y - clip.rect.h, clip.rect.w, clip.rect.h);
			gl.clearColor(1.0, 1.0, 1.0, 1.0);
			gl.clear(gl.COLOR_BUFFER_BIT);
	
			gl.enable(gl.BLEND);
			gl.blendFunc(gl.ONE, gl.ONE_MINUS_SRC_ALPHA);

			if (calls) {
				if (chr.name in calls.chromosomes) {
					for (const record of calls.chromosomes[chr.name].records) {
						let screen_pos = (record.pos - focus) / screen_to_real + canvas.width * 0.5;
						let screen_end = (record.info["END"] - focus) / screen_to_real + canvas.width * 0.5;
						// if (record.alt == "<DEL>")
						// 	ctx.fillStyle = "rgba(200, 10, 10, 0.25)";
						// if (record.alt == "<DUP>")
						// 	ctx.fillStyle = "rgba(10, 200, 10, 0.25)";
						// ctx.fillRect(screen_pos, 0, screen_end - screen_pos, canvas.height);
					}
				}
			}
	
			let top = (clip.view?.top_base ?? 1.0) + (clip.view?.top_scaling ?? 0) * clip.rect.h;
			let bot = (clip.view?.bot_base ?? 0) + (clip.view?.bot_scaling ?? 0) * clip.rect.h;
	
			let scale = 2.0 / (top - bot);
			let offset = -(top + bot) / 2.0;

			const model_view = [
				1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				-focus, offset, 0.0, 1.0
			];

			const proj = [
				2.0 / clip.rect.w / screen_to_real, 0.0, 0.0, 0.0,
				0.0, scale, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			];
	
			if (clip.lines) {
				for (const line of clip.lines) {
					// ctx.save();
					// if (line.style)
					// 	ctx.strokeStyle = line.style;
					// ctx.beginPath();
					// let y = line.y * scale + offset;
					// ctx.moveTo(0, y);
					// ctx.lineTo(canvas.width, y);
					// ctx.stroke();
					// ctx.restore();
				}
			}
	
			for (let data of clip.data) {
				if (data.key) {
					if (data.point_mod) {
						let [visible_chunks, bin_size] = getVisibleChunks(screen_to_real * resolution_modifier * data.point_mod, focus - half_size, focus + half_size);
						drawPointGraph(visible_chunks, model_view, proj, data.key, data.density_key);
					} else {
						drawLineGraph(visible_chunks, model_view, proj, data.key, data.density_key);
						if (data.fill) {
							drawAreaGraph(visible_chunks, model_view, proj, data.fill, data.key, data.density_key);
						}
					}
				}
				if (data.keys)
					drawRectangles(visible_chunks, model_view, proj, data.colors, data.keys, data.density_key);
			}
	
			drawZoomArea(clip);
		}
	}
}
