let vis_data_path = "";

let base_bin_size = 100;
let max_bin_size = 102400;
let bin_sizes = [base_bin_size];

while (bin_sizes[bin_sizes.length - 1] < max_bin_size) {
	bin_sizes.push(bin_sizes[bin_sizes.length - 1] * 2);
}

let chunk_size = 2048;

let resolution_modifier = 1.95;

let chromosome_index = 0;
let screen_to_real = max_bin_size;
let focus = 0;

let current_bin_size = max_bin_size;

let last_chromosome_index = 0;
let last_screen_to_real = 0;
let last_focus = 0;

let fresh_chunk_fetched = true;

let chunks = {};

window.addEventListener('keydown', key_down_handler);
window.addEventListener('keyup', key_up_handler);
window.addEventListener('blur', blur_handler);

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

let zoom_area_start = null;
let zoom_area_end = null;
let last_zoom_area_end = null;

let use_smooth_zoom = true;
let smooth_zoom_delay = 0.5;
let smooth_zoom_target_focus;
let smooth_zoom_target_screen_to_real;
let smooth_zoom_remaining_delay;

function on_mouse_down_canvas(e) {
	if (e.which != 1)
		return;
	zoom_area_start = (e.offsetX - e.target.width * 0.5) * screen_to_real + focus;
	zoom_area_end = zoom_area_start;
	console.log(zoom_area_start);
}

function on_mouse_up_canvas(e) {
	if (e.which != 1 || zoom_area_start === null)
		return;

	zoom_area_end = (e.offsetX - e.target.width * 0.5) * screen_to_real + focus;

	let zoom_area_left = Math.min(zoom_area_start, zoom_area_end);
	let zoom_area_right = Math.max(zoom_area_start, zoom_area_end);
	let zoom_area_width = zoom_area_right - zoom_area_left;
	let zoom_area_midpoint = (zoom_area_left + zoom_area_right) * 0.5;

	if (zoom_area_width / screen_to_real > 2) {
		if (use_smooth_zoom) {
			smooth_zoom_target_focus = zoom_area_midpoint;
			smooth_zoom_target_screen_to_real = zoom_area_width / e.target.width;
			smooth_zoom_remaining_delay = smooth_zoom_delay;
		} else {
			focus = zoom_area_midpoint;
			screen_to_real = zoom_area_width / e.target.width;
		}
	}

	zoom_area_start = null;
	zoom_area_end = null;
}

function on_mouse_move(e) {
	if (zoom_area_start === null)
		return;

	const canvas = document.getElementById("view_canvas");
	zoom_area_end = (e.clientX - canvas.getBoundingClientRect().left - canvas.width * 0.5) * screen_to_real + focus;
}

function check_update() {
	let needed_bin_size = screen_to_real * resolution_modifier;
	current_bin_size = base_bin_size;
	for (let i = 0; i < bin_sizes.length; ++i) {
		if (bin_sizes[i] <= needed_bin_size)
			current_bin_size = bin_sizes[i];
		else
			break;
	}
	if (last_chromosome_index != chromosome_index 
		|| last_screen_to_real != screen_to_real 
		|| last_focus != focus 
		|| fresh_chunk_fetched
		|| last_zoom_area_end != zoom_area_end) {

		last_chromosome_index = chromosome_index;
		last_screen_to_real = screen_to_real;
		last_focus = focus;
		fresh_chunk_fetched = false;
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

let last_chromosome_name;

function render(render_bin_size) {
	const canvas = document.getElementById("view_canvas");
	const ctx = canvas.getContext("2d");

	let current_chromosome = chromosome_names[chromosome_index];
	if (current_chromosome != last_chromosome_name) {
		document.getElementsByTagName("H1")[0].innerHTML = current_chromosome;
		last_chromosome_name = current_chromosome;
	}

	if (render_bin_size === undefined)
		render_bin_size = current_bin_size;

	// maybe we should unify log2_offset and baf_offset
	let chr_offset = chromosome_offsets[chromosome_index];
	let chr_size = chromosome_sizes[chromosome_index];
	let max_chunk_index = Math.floor(chr_size / (chunk_size * render_bin_size));

	let visible_chunks = [];
	let first_index = Math.floor((focus - canvas.width * 0.5 * screen_to_real - chr_offset) / (chunk_size * render_bin_size));
	first_index = Math.max(0, first_index);
	let last_index = Math.floor((focus + canvas.width * 0.5 * screen_to_real - chr_offset) / (chunk_size * render_bin_size));
	last_index = Math.min(max_chunk_index, last_index);
	for (let i = first_index; i <= last_index; ++i) {
		visible_chunks.push(get_chunk(current_chromosome, render_bin_size, i));
	}

	if (first_index > 0) {
		let look_ahead_index = first_index - 1;
		get_chunk(current_chromosome, render_bin_size, look_ahead_index);
	}
	if (last_index < max_chunk_index) {
		let look_ahead_index = last_index + 1;
		get_chunk(current_chromosome, render_bin_size, look_ahead_index);
	}

	for (let i = 0; i < visible_chunks.length; ++i) {
		if (visible_chunks[i] == null) {
			// (old todo) partition the rendering to prevent entire view from dropping to a lower resolution
			// fixed by using look ahead
			if (render_bin_size < max_bin_size)
				render(render_bin_size * 2);
			return;
		}
	}

	if (canvas.width != window.innerWidth)
		canvas.width = window.innerWidth;

	ctx.setTransform(1, 0, 0, 1, 0, 0);
	ctx.clearRect(0, 0, canvas.width, canvas.height);

	function drawGraph(key, x, x_scale, y, y_scale, density_key) {
		ctx.beginPath();
		ctx.moveTo(x, y + visible_chunks[0].data[key][0] * y_scale);

		for (let j = 0; j < visible_chunks.length; ++j) {
			let data = visible_chunks[j].data[key];
			for (let i = 0; i < data.length; ++i) {
				let value = data[i];
				ctx.lineTo(x + i * x_scale, y + value * y_scale);
			}
			x += data.length * x_scale;
		}

		ctx.stroke();
	}

	let x_start = (visible_chunks[0].log2_offset - focus) / screen_to_real + canvas.width * 0.5;

	drawGraph("log2_density", x_start, render_bin_size / screen_to_real, 50, -40);

	drawGraph("log2_min", x_start, render_bin_size / screen_to_real, 150, -30);
	drawGraph("log2_mean", x_start, render_bin_size / screen_to_real, 150, -30);
	drawGraph("log2_max", x_start, render_bin_size / screen_to_real, 150, -30);

	ctx.fillStyle = "rgba(100, 100, 240, 0.5)";
	
	let zoom_area_screen_start = (zoom_area_start - focus) / screen_to_real + canvas.width * 0.5;
	let zoom_area_screen_end = (zoom_area_end - focus) / screen_to_real + canvas.width * 0.5;
	let zoom_area_screen_left = Math.min(zoom_area_screen_start, zoom_area_screen_end);
	let zoom_area_screen_right = Math.max(zoom_area_screen_start, zoom_area_screen_end);
	let zoom_area_screen_width = zoom_area_screen_right - zoom_area_screen_left;
	ctx.fillRect(zoom_area_screen_left, 0, zoom_area_screen_width, canvas.height);
}

function get_chunk(chr, size, index) {
	let chunk_identifier = chr + "_" + size + "_" + index;
	// console.log(chunk_identifier);
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

		let view = new DataView(arrayBuffer);
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
		}

		fresh_chunk_fetched = true;
	});
}

window.onload = function () {
	document.getElementById("view_canvas").addEventListener("mousedown", on_mouse_down_canvas);
	document.getElementById("view_canvas").addEventListener("mouseup", on_mouse_up_canvas);
	window.addEventListener("mousemove", on_mouse_move);

	window.requestAnimationFrame(on_animation_frame);
};
