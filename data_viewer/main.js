let vis_data_path = "";

let base_bin_size = 100;
let max_bin_size = 102400;
let bin_sizes = [base_bin_size];

while (bin_sizes[bin_sizes.length - 1] < max_bin_size) {
	bin_sizes.push(bin_sizes[bin_sizes.length - 1] * 2);
}

let resolution_modifier = 1.95;
let initial_view_margin = 1.2;
let min_zoom_area_width = 5;

let chr = {};

let chromosome_index = 0;
let screen_to_real = max_bin_size;
let focus = 0;

let current_bin_size = max_bin_size;

let last_chromosome_index;
let last_screen_to_real;
let last_focus;

let render_out_of_date = true;

let chunks = {};

let calls;

window.addEventListener('keydown', key_down_handler);
window.addEventListener('keyup', key_up_handler);
window.addEventListener('blur', blur_handler);
window.addEventListener('hashchange', hash_change_handler);

let keys_down = {};

function key_down_handler(e) {
	keys_down[e.key] = true;

	if (e.key == "q")
		chromosome_index -= 1;
	if (e.key == "e")
		chromosome_index += 1;
	if (chromosome_index < 0)
		chromosome_index = 0;
	if (chromosome_index >= chromosome_names.length)
		chromosome_index = chromosome_names.length - 1;
}

function key_up_handler(e) {
	keys_down[e.key] = false;
}

function blur_handler(e) {
	keys_down = {};
	zoom_area_start = null;
	zoom_area_end = null;
}

function hash_change_handler(e) {
	if (get_hash_from_state() != window.location.hash) {
		set_state_from_hash(window.location.hash);
		window.location.hash = get_hash_from_state();
	}
}

let zoom_area_start = null;
let zoom_area_end = null;
let last_zoom_area_end = null;
let view_mouse_x = null;

let use_smooth_zoom = true;
let smooth_zoom_delay = 0.5;
let smooth_zoom_target_focus;
let smooth_zoom_target_screen_to_real;
let smooth_zoom_remaining_delay;

function on_mouse_down_canvas(e) {
	if (e.which != 1)
		return;
	if (e.target.tagName != "CANVAS")
		return;
	zoom_area_start = (e.clientX - document.body.clientWidth * 0.5) * screen_to_real + focus;
	zoom_area_end = zoom_area_start;
	// console.log(zoom_area_start);

	if (calls) {
		if (chr.name in calls.chromosomes) {
			for (const record of calls.chromosomes[chr.name].records) {
				let screen_pos = (record.pos - focus) / screen_to_real + document.body.clientWidth * 0.5;
				let screen_end = (record.info["END"] - focus) / screen_to_real + document.body.clientWidth * 0.5;
				if (e.clientX > screen_pos && e.clientX < screen_end) {
					// console.log(record);
				}
			}
		}
	}
}

function on_mouse_up_canvas(e) {
	if (e.which != 1 || zoom_area_start === null)
		return;

	zoom_area_end = (e.clientX - document.body.clientWidth * 0.5) * screen_to_real + focus;

	if (Math.abs(zoom_area_end - zoom_area_start) / screen_to_real >= min_zoom_area_width) {
		perform_zoom(zoom_area_start, zoom_area_end);
	}

	zoom_area_start = null;
	zoom_area_end = null;
}

function on_mouse_move(e) {
	view_mouse_x = e.clientX;
	if (zoom_area_start === null)
		return;
	zoom_area_end = (e.clientX - document.body.clientWidth * 0.5) * screen_to_real + focus;
}

function perform_zoom(start, end, smooth) {
	let zoom_area_left = Math.min(start, end);
	let zoom_area_right = Math.max(start, end);
	let zoom_area_width = zoom_area_right - zoom_area_left;
	let zoom_area_midpoint = (zoom_area_left + zoom_area_right) * 0.5;

	smooth = smooth ?? use_smooth_zoom;

	// if (zoom_area_width / screen_to_real >= 1)
	smooth_zoom_target_focus = zoom_area_midpoint;
	smooth_zoom_target_screen_to_real = zoom_area_width / document.body.clientWidth;
	if (smooth) {
		smooth_zoom_remaining_delay = smooth_zoom_delay;
	} else {
		focus = smooth_zoom_target_focus;
		screen_to_real = smooth_zoom_target_screen_to_real;
	}
}

function get_start_and_end(smooth) {
	// if we want smooth coordinates, we return the current start and end
	// ( which might be in the middle of a smooth zoom )
	if (smooth) {
		let half = screen_to_real * document.body.clientWidth * 0.5;
		return [focus - half, focus + half];
	} else {
		let half = smooth_zoom_target_screen_to_real * document.body.clientWidth * 0.5;
		return [smooth_zoom_target_focus - half, smooth_zoom_target_focus + half];
	}
}

function get_hash(start, end) {
	let hash = "#" + chromosome_names[chromosome_index];
	if (start > chromosome_offsets[chromosome_index] || end < chromosome_sizes[chromosome_index] + chromosome_offsets[chromosome_index])
		hash += ":" + Math.floor(start) + "-" + Math.floor(end);
	return hash;
}

function get_hash_from_state() {
	const [start, end] = get_start_and_end();
	return get_hash(start, end);
}

function set_state_from_hash(hash) {
	let re = /#([^:]+)(:(-?\d+)-(\d+))?/;
	let matches = hash.match(re);
	if (matches) {
		chromosome_index = chromosome_names.findIndex(name => name == matches[1]);
		const same_index = chromosome_index == chr.index;
		if (matches[2])
			perform_zoom(matches[3], matches[4], same_index && undefined);
		else {
			let midpoint = chromosome_offsets[chromosome_index] + chromosome_sizes[chromosome_index] * 0.5;
			let half_size = chromosome_sizes[chromosome_index] * initial_view_margin * 0.5;
			perform_zoom(midpoint - half_size, midpoint + half_size, same_index && undefined);
		}
	}
}

function check_update() {

	if (last_chromosome_index != chromosome_index 
		|| last_screen_to_real != screen_to_real 
		|| last_focus != focus 
		|| render_out_of_date 
		|| last_zoom_area_end != zoom_area_end) {

		if (last_chromosome_index != chromosome_index) {
			document.getElementById("chromosome_dropdown_select").selectedIndex = chromosome_index;
			let chromosome_dropdown_current = document.getElementById("chromosome_dropdown_current");
			chromosome_dropdown_current.innerHTML = chromosome_names[chromosome_index];
			chromosome_dropdown_current.href = "#" + chromosome_names[chromosome_index];
		}

		let hash = get_hash_from_state();
		if (hash != undefined && hash != window.location.hash)
			window.location.hash = hash;
		let current_chromosome_dropdown_a = document.getElementById("chromosome_dropdown_content").children[chromosome_index];
		if (current_chromosome_dropdown_a.href != hash)
			current_chromosome_dropdown_a.href = hash;

		chr.index = chromosome_index;
		chr.name = chromosome_names[chr.index];
		chr.offset = chromosome_offsets[chr.index];
		chr.size = chromosome_sizes[chr.index];

		last_chromosome_index = chromosome_index;
		last_screen_to_real = screen_to_real;
		last_focus = focus;
		render_out_of_date = false;
		last_zoom_area_end = zoom_area_end;
		render();
	}
}

let last_timestamp;
function on_animation_frame(timestamp) {
	let elapsed = 0.0;
	if (last_timestamp !== undefined)
		elapsed = (timestamp - last_timestamp) * 0.001;
	elapsed = Math.min(0.33, elapsed);
	last_timestamp = timestamp;

	if (keys_down["w"])
		screen_to_real *= Math.exp(Math.log(0.5) * elapsed);
	if (keys_down["a"])
		focus -= 400 * screen_to_real * elapsed;
	if (keys_down["s"])
		screen_to_real *= Math.exp(Math.log(2.0) * elapsed);
	if (keys_down["d"])
		focus += 400 * screen_to_real * elapsed;

	if (use_smooth_zoom) {
		if (smooth_zoom_remaining_delay > 0) {
			let last_remaining_delay = smooth_zoom_remaining_delay;
			smooth_zoom_remaining_delay -= elapsed;
			smooth_zoom_remaining_delay = Math.max(0, smooth_zoom_remaining_delay);
			let linear_t = 1.0 - smooth_zoom_remaining_delay / smooth_zoom_delay;
			let last_linear_t = 1.0 - last_remaining_delay / smooth_zoom_delay;
			let relative_linear_t = (linear_t - last_linear_t) / (1.0 - last_linear_t);

			if (linear_t == 1.0) {
				focus = smooth_zoom_target_focus;
				screen_to_real = smooth_zoom_target_screen_to_real;
			} else {
				let og_rel = Math.exp(Math.log(smooth_zoom_target_screen_to_real / screen_to_real) / (1.0 - last_linear_t));
				og_rel = Math.max(1.0 / 2000.0, og_rel);
				//console.log(og_rel);
				let log_t = (1.0 - Math.exp(Math.log(og_rel) * linear_t)) / (1.0 - og_rel);
				let last_log_t = (1.0 - Math.exp(Math.log(og_rel) * last_linear_t)) / (1.0 - og_rel);
				let relative_log_t = (log_t - last_log_t) / (1.0 - last_log_t);

				focus = focus * (1.0 - relative_log_t) + smooth_zoom_target_focus * relative_log_t;
				//focus = focus * (1.0 - relative_linear_t) + smooth_zoom_target_focus * relative_linear_t;
				screen_to_real *= Math.exp(Math.log(smooth_zoom_target_screen_to_real / screen_to_real) * relative_linear_t);
				//screen_to_real = screen_to_real * t + smooth_zoom_target_screen_to_real * (1.0 - t);
			}
		}
	}

	focus = Math.max(chromosome_offsets[chromosome_index], Math.min(chromosome_offsets[chromosome_index] + chromosome_sizes[chromosome_index], focus));
	screen_to_real = Math.max(base_bin_size / 10, Math.min(max_bin_size * 10, screen_to_real));

	window.requestAnimationFrame(on_animation_frame);
	check_update();
}

// very temp initialization
let views = [
	{
		type: "chromosome",
		height: 450,
		clips: [
			{
				height: 50,
				label: "Base Densities",
				data: [
					{
						keys: [
							"base_a_density",
							"base_c_density",
							"base_g_density",
							"base_t_density"
						],
						colors: [
							[0.2, 0.0, 0.0],
							[1.0, 1.0, 0.0],
							[0.0, 1.0, 0.0],
							[0.0, 0.0, 0.2],
						],
						labels: [
							"A",
							"C",
							"G",
							"T"
						],
						density_key: "base_total_density"
					}
				]
			},
			{
				height: 50,
				label: "Coverage Density",
				data: [
					{
						key: "coverage_density",
						y_offset: 5,
						y_scale: 40
					}
				]
			},
			{
				height: 200,
				label: "Coverage (Log2)",
				data: [
					{
						key: "coverage_min",
						density_key: "coverage_density",
						y_offset: 120,
						y_scale: 30
					},
					{
						key: "coverage_mean",
						density_key: "coverage_density",
						y_offset: 120,
						y_scale: 30
					},
					{
						key: "coverage_max",
						density_key: "coverage_density",
						y_offset: 120,
						y_scale: 30
					}
				],
				lines: [
					{
						y: 120,
						style: "rgba(50, 50, 200, 0.5)"
					}
				]
			},
			{
				height: 50,
				label: "BAF Mean",
				data: [
					{
						key: "baf_mid_mean",
						density_key: "baf_mid_density",
						y_offset: 0,
						y_scale: 50
					}
				]
			},
			{
				height: 50,
				label: "BAF Top Density",
				data: [
					{
						key: "baf_top_density",
						y_offset: 0,
						y_scale: 50
					}
				]
			},
			{
				height: 50,
				label: "BAF Main Density",
				data: [
					{
						key: "baf_mid_density",
						y_offset: 0,
						y_scale: 50
					}
				]
			},
			{
				height: 50,
				label: "BAF Bottom Density",
				data: [
					{
						key: "baf_bot_density",
						y_offset: 0,
						y_scale: 50
					}
				]
			}
		]
	},
	{
		type: "overview",
		height: 80,
		clips: [
			{
				height: 80,
				label: "Overview"
			}
		]
	}
];

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

function amplify_density_values(values) {
	values = values.map(e => Math.pow(e, 5));
	let sum = values.reduce((acc, e) => acc + e, 0);
	return values.map(e => e / sum);
}

let last_visible_chunks;

function render(render_bin_size) {

	if (render_bin_size === undefined) {
		
		let needed_bin_size = screen_to_real * resolution_modifier;
		current_bin_size = base_bin_size;
		for (let i = 0; i < bin_sizes.length; ++i) {
			if (bin_sizes[i] <= needed_bin_size)
				current_bin_size = bin_sizes[i];
			else
				break;
		}

		render_bin_size = current_bin_size;
	}

	let max_chunk_index = Math.floor(chr.size / (chunk_size * render_bin_size));

	let visible_chunks = [];
	let first_index = Math.floor((focus - document.body.clientWidth * 0.5 * screen_to_real - chr.offset) / (chunk_size * render_bin_size));
	first_index = Math.max(0, first_index);
	let last_index = Math.floor((focus + document.body.clientWidth * 0.5 * screen_to_real - chr.offset) / (chunk_size * render_bin_size));
	last_index = Math.min(max_chunk_index, last_index);
	for (let i = first_index; i <= last_index; ++i) {
		visible_chunks.push(get_chunk(chr.name, render_bin_size, i));
	}

	if (first_index > 0) {
		let look_ahead_index = first_index - 1;
		get_chunk(chr.name, render_bin_size, look_ahead_index);
	}
	if (last_index < max_chunk_index) {
		let look_ahead_index = last_index + 1;
		get_chunk(chr.name, render_bin_size, look_ahead_index);
	}

	for (let i = 0; i < visible_chunks.length; ++i) {
		if (visible_chunks[i] == null) {
			if (render_bin_size < max_bin_size)
				render(render_bin_size * 2);
			return;
		}
	}

	last_visible_chunks = last_visible_chunks ?? visible_chunks;
	let visible_chunks_changed = !visible_chunks.every((e, i) => e === last_visible_chunks[i]);
	last_visible_chunks = visible_chunks;

	function drawGraph(path, key, x_off, x_scale, y_off, y_scale, density_key) {
		// ctx.beginPath();

		let strokes = 0;
		let x;
		let y;

		for (let j = 0; j < visible_chunks.length; ++j) {
			let data = visible_chunks[j].data[key];
			if (!data)
				return;
			let density_data;
			if (density_key !== undefined)
				density_data = visible_chunks[j].data[density_key];
			for (let i = 0; i < data.length; ++i) {
				let value = data[i];
				if (density_data)
					if (density_data[i] == 0) {
						if (strokes == 1) {
							path.lineTo(x + 0.5 * x_scale, y);
							path.lineTo(x - 0.5 * x_scale, y);
						}
						strokes = 0;
						continue;
					}
				x = x_off + i * x_scale;
				y = y_off + value * y_scale;
				if (strokes > 0)
					path.lineTo(x, y);
				else
					path.moveTo(x, y);
				++strokes;
			}
			x_off += data.length * x_scale;
		}

		// ctx.stroke();
	}

	function drawRectangles(ctx, keys, colors, x_off, x_scale, height, density_key) {

		let x;

		for (let j = 0; j < visible_chunks.length; ++j) {
			let data = keys.map(e => visible_chunks[j].data[e]);
			if (data.some(e => !e))
				return;
			let density_data;
			if (density_key !== undefined)
				density_data = visible_chunks[j].data[density_key];
			for (let i = 0; i < data[0].length; ++i) {
				let values = data.map(e => e[i]);
				if (density_data)
					if (density_data[i] != 0)
						values = values.map(e => e / density_data[i]);
					else
						continue;
				values = amplify_density_values(values);
				let color = colors.map((e, i) => e.map(e => e * values[i]))
					.reduce((acc, c) => acc.map((e, i) => e + c[i]), [0, 0, 0]);
				//color = xyz_to_srgb8(color);
				color = rgb_to_srgb(color);
				color = color.map(e => Math.floor(e * 255));
				x = x_off + i * x_scale;
				ctx.fillStyle = `rgba(${color[0]}, ${color[1]}, ${color[2]}, 1.0)`;
				ctx.fillRect(x, 0, x_scale * 2.0, height);
			}
			x_off += data[0].length * x_scale;
		}
	}

	let x_start = (visible_chunks[0].offset - focus) / screen_to_real + document.body.clientWidth * 0.5;

	let view = views[0];
	for (const clip of view.clips) {

		let canvas = clip.canvas;

		if (canvas.width != document.body.clientWidth)
			canvas.width = document.body.clientWidth;

		let ctx = canvas.getContext("2d");

		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, canvas.width, canvas.height);

		if (calls) {
			if (chr.name in calls.chromosomes) {
				for (const record of calls.chromosomes[chr.name].records) {
					let screen_pos = (record.pos - focus) / screen_to_real + canvas.width * 0.5;
					let screen_end = (record.info["END"] - focus) / screen_to_real + canvas.width * 0.5;
					if (record.alt == "<DEL>")
						ctx.fillStyle = "rgba(200, 10, 10, 0.25)";
					if (record.alt == "<DUP>")
						ctx.fillStyle = "rgba(10, 200, 10, 0.25)";
					ctx.fillRect(screen_pos, 0, screen_end - screen_pos, canvas.height);
				}
			}
		}

		if (clip.lines) {
			for (const line of clip.lines) {
				ctx.save();
				if (line.style)
					ctx.strokeStyle = line.style;
				ctx.beginPath();
				ctx.moveTo(0, clip.height - line.y);
				ctx.lineTo(canvas.width, clip.height - line.y);
				ctx.stroke();
				ctx.restore();
			}
		}
		
		clip.path = new Path2D();

		for (let data of clip.data) {
			if (data.key)
				drawGraph(clip.path, data.key, x_start, render_bin_size / screen_to_real, clip.height - data.y_offset, -data.y_scale, data.density_key);
			if (data.keys)
				drawRectangles(ctx, data.keys, data.colors, x_start, render_bin_size / screen_to_real, canvas.height, data.density_key);
		}

		ctx.stroke(clip.path);

		if (zoom_area_start) {
			let zoom_area_screen_start = (zoom_area_start - focus) / screen_to_real + canvas.width * 0.5;
			let zoom_area_screen_end = view_mouse_x;
			let zoom_area_screen_left = Math.min(zoom_area_screen_start, zoom_area_screen_end);
			let zoom_area_screen_right = Math.max(zoom_area_screen_start, zoom_area_screen_end);
			let zoom_area_screen_width = zoom_area_screen_right - zoom_area_screen_left;
			ctx.fillStyle = "rgba(100, 100, 240, 0.5)";
			if (zoom_area_screen_width < min_zoom_area_width)
				ctx.fillStyle = "rgba(100, 100, 240, 0.25)";
			ctx.fillRect(zoom_area_screen_left, 0, zoom_area_screen_width, canvas.height);
		}
	}

	{
		let clip = views[1].clips[0];
		let canvas = clip.canvas;
		let ctx = canvas.getContext("2d");

		if (canvas.width != document.body.clientWidth)
			canvas.width = document.body.clientWidth;

		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, canvas.width, canvas.height);

		let y_top = 0;
		let height = clip.height;
		let y_bot = y_top + height;

		ctx.beginPath();
		let total_size = chromosome_sizes.reduce((a, e) => a + e);
		let x = 0;
		let [start, end] = get_start_and_end(smooth = true);
		start = Math.max(0, start - chr.offset) / chr.size;
		end = Math.min(chr.size, end - chr.offset) / chr.size;
		for (let i = 0; i < chromosome_names.length - 1; ++i) {
			let stride = chromosome_sizes[i] * document.body.clientWidth / total_size;
			if (i == chr.index) {
				start = x + stride * start;
				end = x + stride * end;
			}
			x += stride;
			ctx.moveTo(x, y_top);
			ctx.lineTo(x, y_bot);
		}
		ctx.stroke();

		ctx.fillStyle = "rgba(100, 100, 240, 0.5)";
		ctx.fillRect(start, y_top, end - start, height);
	}

	/*{
		ctx.beginPath();
		let y = 0;
		for (const view of views) {
			if (view.clips) {
				let clip_y = y;
				for (const clip of view.clips) {
					clip_y += clip.height;
					ctx.moveTo(0, clip_y);
					ctx.lineTo(canvas.width, clip_y);
				}
			}
			y += view.height;
			ctx.moveTo(0, y);
			ctx.lineTo(canvas.width, y);
		}
		ctx.stroke();
	}*/
}

function get_chunk(chr, size, index) {
	let chunk_identifier = chr + "_" + size + "_" + index;
	//console.log(chunk_identifier);
	if (chunk_identifier in chunks)
		return chunks[chunk_identifier];
	else {
		chunks[chunk_identifier] = null;
		fetch_chunk(chr, size, index);
		return null;
	}
}

function fetch_chunk(chr, size, index) {
	let chunk_identifier = chr + "_" + size + "_" + index;
	fetch(vis_data_path + chunk_identifier + ".cvd")
	.then(response => {
		if (response.ok)
			return response.arrayBuffer()
		else
			return null;
	})
	.then(arrayBuffer => {
		if (arrayBuffer === null)
			return;

		let chunk = {};
		chunks[chunk_identifier] = chunk;

		chunk.offset = chromosome_offsets[chromosome_names.findIndex(e => e == chr)] + size * index * chunk_size;
		chunk.data = {};

		let offset = 0;
		for (const data_line of chunk_data_lines) {
			chunk.data[data_line] = new Float32Array(arrayBuffer, offset, chunk_size);
			offset += chunk_size * 4;
		}

		// console.log(chunk);

		/* let view = new DataView(arrayBuffer);
		let scale = view.getInt32(0, true);
		let baf_offset = view.getInt32(4, true);
		let baf_size = view.getInt32(8, true);

		let off = 12 + baf_size * 20;

		let log2_offset = view.getInt32(off, true);
		let log2_size = view.getInt32(off + 4, true);

		chunk.baf_offset = baf_offset;
		chunk.baf_size = baf_size;
		chunk.log2_offset = log2_offset;
		chunk.log2_size = log2_size;

		chunk.data = {}

		let log2_density = [];
		let log2_min = [];
		let log2_mean = [];
		let log2_max = [];

		chunk.data["log2_density"] = log2_density;
		chunk.data["log2_min"] = log2_min;
		chunk.data["log2_mean"] = log2_mean;
		chunk.data["log2_max"] = log2_max;

		for (let i = 0; i < log2_size; ++i) {
			log2_density.push(view.getFloat32(off + 8 + i * 16 + 0, true));
			log2_min.push(view.getFloat32(off + 8 + i * 16 + 4, true));
			log2_mean.push(view.getFloat32(off + 8 + i * 16 + 8, true));
			log2_max.push(view.getFloat32(off + 8 + i * 16 + 12, true));
		} */

		render_out_of_date = true;

		// console.log(chunk_identifier);
	});
}

function rgb_to_hex(color) {
	return "#" + (1 << 24 | color[0] << 16 | color[1] << 8 | color[2]).toString(16).slice(1);
}

function hex_to_rgb(color) {
	return [
		parseInt(color.slice(1, 3), 16),
		parseInt(color.slice(3, 5), 16),
		parseInt(color.slice(5, 7), 16)
	];
}

window.onload = function () {
	window.addEventListener("mousedown", on_mouse_down_canvas);
	window.addEventListener("mouseup", on_mouse_up_canvas);
	window.addEventListener("mousemove", on_mouse_move);

	let views_element = document.getElementById("views");
	for (let view of views) {
		for (let clip of view.clips ?? []) {
			clip.wrapper = document.createElement("div");
			clip.wrapper.className = "canvas_wrapper";
			views_element.appendChild(clip.wrapper);

			clip.canvas = document.createElement("canvas");
			clip.canvas.height = clip.height;
			clip.canvas.className = "canvas_cursor_text";
			clip.wrapper.appendChild(clip.canvas);
			
			let label = document.createElement("a");
			label.innerHTML = clip.label;
			label.className = "label";
			clip.wrapper.appendChild(label);

			if (clip.data) {
				for (let data of clip.data) {
					if (data.colors) {
						for (let i = 0; i < data.colors.length; ++i) {
							let input = document.createElement("input");
							input.type = "color";
							input.value = rgb_to_hex(data.colors[i].map(e => Math.floor(e * 255)));
							input.oninput = (ev) => {
								data.colors[i] = hex_to_rgb(ev.target.value).map(e => e / 255);
								render_out_of_date = true;
							};
							label.appendChild(input);
						}
					}
				}
			}
		}
	}

	let chromosome_select = document.getElementById("chromosome_dropdown_select");
	chromosome_select.onchange = function () {
		chromosome_index = chromosome_select.selectedIndex;
	};
	hash_change_handler();

	fetch("calls.json")
	.then(response => response.json())
	.then(data => {
		calls = data;
		render_out_of_date = true;
	});

	window.requestAnimationFrame(on_animation_frame);
};
