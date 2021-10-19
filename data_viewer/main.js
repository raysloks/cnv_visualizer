let chunk_path = "chunks/";

let bin_sizes;

let resolution_modifier = 2.0;
let initial_view_margin = 1.2;
let min_zoom_area_width = 5;

let chr = {};

let chromosome_index = null;
let screen_to_real = 1;
let focus = 0;

let last_chromosome_index = null;
let last_screen_to_real;
let last_focus;

let canvas;
let canvas_wrapper;

let render_out_of_date = true;

let calls;

window.addEventListener('keydown', key_down_handler);
window.addEventListener('keyup', key_up_handler);
window.addEventListener('blur', blur_handler);
window.addEventListener('hashchange', hash_change_handler);

let keys_down = {};

function key_down_handler(e) {
	keys_down[e.key] = true;
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

function perform_zoom(start, end, smooth) {
	let zoom_area_left = Math.min(start, end);
	let zoom_area_right = Math.max(start, end);
	let zoom_area_width = zoom_area_right - zoom_area_left;
	let zoom_area_midpoint = (zoom_area_left + zoom_area_right) * 0.5;

	smooth = smooth ?? use_smooth_zoom;

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
	if (chromosome_index === null)
		return "";
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
	} else {
		chromosome_index = null;
	}
}

function check_update() {

	let hash = get_hash_from_state();
	if (hash != undefined && hash != window.location.hash)
		window.location.hash = hash;

	if (last_chromosome_index != chromosome_index 
		|| last_screen_to_real != screen_to_real 
		|| last_focus != focus 
		|| render_out_of_date 
		|| last_zoom_area_end != zoom_area_end) {

		if (last_chromosome_index != chromosome_index) {
			let chromosome_dropdown_current = document.getElementById("chromosome_dropdown_current");
			chromosome_dropdown_current.innerHTML = chromosome_names[chromosome_index];
			chromosome_dropdown_current.href = "#" + chromosome_names[chromosome_index];
		}
		
		if (chromosome_index !== null) {
			let current_chromosome_dropdown_a = document.getElementById("chromosome_dropdown_content").children[chromosome_index];
			if (current_chromosome_dropdown_a.href != hash)
				current_chromosome_dropdown_a.href = hash;
			// this is super scuffed
			let clip = clips.find(e => e.type == "overview" && e.subclips);
			let current_chromosome_overview_a = clip.subclips[chromosome_index].render_area.parentNode;
			if (current_chromosome_overview_a.href != hash) {
				current_chromosome_overview_a.href = hash;
			}
			{
				// if you change chromosome in the middle of a smooth zoom, this won't match the href. TODO fix
				let zoom_overlay = current_chromosome_overview_a.querySelector(".overview_zoom_overlay");
				let [start, end] = get_start_and_end(smooth = true);
				let chr_midpoint = chromosome_offsets[chromosome_index] + chromosome_sizes[chromosome_index] * 0.5;
				start = Math.max(0, (start - chr_midpoint) / initial_view_margin + chr_midpoint - chromosome_offsets[chromosome_index]);
				end = Math.min(chromosome_sizes[chromosome_index], (end - chr_midpoint) / initial_view_margin + chr_midpoint - chromosome_offsets[chromosome_index]);
				zoom_overlay.style.left = start / chromosome_sizes[chromosome_index] * 100 + "%";
				zoom_overlay.style.right = (1 - end / chromosome_sizes[chromosome_index]) * 100 + "%";
				//zoom_overlay.style.display = hash.includes(":") ? "block" : "none";
			}
			current_chromosome_overview_a.classList.add("overview_current");
			if (last_chromosome_index !== chromosome_index && last_chromosome_index !== null) {
				let last_chromosome_overview_a = clip.subclips[last_chromosome_index].render_area.parentNode;
				last_chromosome_overview_a.classList.remove("overview_current");
			}
		}

		chr.index = chromosome_index;
		chr.name = chromosome_names[chr.index];
		chr.offset = chromosome_offsets[chr.index];
		chr.size = chromosome_sizes[chr.index];

		last_chromosome_index = chromosome_index;
		last_screen_to_real = screen_to_real;
		last_focus = focus;
		render_out_of_date = false;
		last_zoom_area_end = zoom_area_end;
		render(true);
	} else {
		render(false);
	}
}

function tick_smooth_zoom(elapsed) {
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
			if (screen_to_real != smooth_zoom_target_screen_to_real) {
				let og_rel = Math.exp(Math.log(smooth_zoom_target_screen_to_real / screen_to_real) / (1.0 - last_linear_t));
				og_rel = Math.max(1.0 / 20000.0, og_rel);
				let log_t = (1.0 - Math.exp(Math.log(og_rel) * linear_t)) / (1.0 - og_rel);
				let last_log_t = (1.0 - Math.exp(Math.log(og_rel) * last_linear_t)) / (1.0 - og_rel);
				let relative_log_t = (log_t - last_log_t) / (1.0 - last_log_t);
	
				focus = focus * (1.0 - relative_log_t) + smooth_zoom_target_focus * relative_log_t;
				screen_to_real *= Math.exp(Math.log(smooth_zoom_target_screen_to_real / screen_to_real) * relative_linear_t);
			} else {
				focus = focus * (1.0 - relative_linear_t) + smooth_zoom_target_focus * relative_linear_t;
			}
		}
	}
}

let last_timestamp;
function on_animation_frame(timestamp) {
	let elapsed = 0.0;
	if (last_timestamp !== undefined)
		elapsed = (timestamp - last_timestamp) * 0.001;
	elapsed = Math.min(0.33, elapsed);
	last_timestamp = timestamp;

	if (use_smooth_zoom) {
		tick_smooth_zoom(elapsed);
	}

	focus = Math.max(chromosome_offsets[chromosome_index], Math.min(chromosome_offsets[chromosome_index] + chromosome_sizes[chromosome_index], focus));
	screen_to_real = Math.max(base_bin_size / 100, Math.min(max_bin_size * 10, screen_to_real));

	window.requestAnimationFrame(on_animation_frame);
	check_update();
}

// very temp initialization
let clips = [
	{
		height: 80,
		label: "Overview",
		type: "overview",
		data: [
			{
				keys: [
					"base_a_density",
					"base_c_density",
					"base_g_density",
					"base_t_density"
				],
				colors: [
					[0.2, 0.0, 0.0, 1.0],
					[1.0, 1.0, 0.0, 1.0],
					[0.0, 1.0, 0.0, 1.0],
					[0.0, 0.0, 0.2, 1.0]
				],
				labels: [
					"A",
					"C",
					"G",
					"T"
				],
				amplification: 5,
				density_key: "base_total_density"
			}
		]
	},
	{
		height: 40,
		type: "overview"
	},
	{
		height: 50,
		label: "Raw Base Densities",
		data: [
			{
				keys: [
					"base_a_density",
					"base_c_density",
					"base_g_density",
					"base_t_density"
				],
				colors: [
					[0.2, 0.0, 0.0, 1.0],
					[1.0, 1.0, 0.0, 1.0],
					[0.0, 1.0, 0.0, 1.0],
					[0.0, 0.0, 0.2, 1.0]
				],
				labels: [
					"A",
					"C",
					"G",
					"T"
				],
				amplification: 5,
				density_key: "base_total_density"
			}
		]
	},
	{
		height: 50,
		label: "BAF Undersampled",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_undersampled",
				density_key: "baf_density",
				point_mod: 1 / 16
			}
		]
	},
	{
		height: 100,
		label: "Coverage Points (Log2)",
		view: {
			top_base: 0.75,
			bot_base: -0.75,
			top_scaling: 0.0075,
			bot_scaling: -0.0075
		},
		data: [
			{
				key: "coverage_mean_log2",
				density_key: "coverage_density",
				point_mod: 1 / 16
			}
		],
		lines: [
			{
				y: 0,
				style: "rgba(50, 50, 200, 0.5)"
			}
		]
	},
	{
		height: 100,
		type: "annotation",
		label: "Ensembl",
		source: "../../../ensembl/"
	},
	{
		height: 50,
		label: "Coverage Density",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "coverage_density",
				fill: [0.05, 0.05, 0.85, 0.5]
			}
		]
	},
	{
		height: 150,
		label: "Coverage (Log2)",
		view: {
			top_base: 0.75,
			bot_base: -1.25,
			top_scaling: 0.0075,
			bot_scaling: -0.0125
		},
		data: [
			{
				key: "coverage_min_log2",
				density_key: "coverage_density"
			},
			{
				key: "coverage_mean_log2",
				density_key: "coverage_density"
			},
			{
				key: "coverage_max_log2",
				density_key: "coverage_density"
			},
			{
				key: "coverage_mean_log2_negative",
				density_key: "coverage_density_negative"
			},
			{
				key: "coverage_mean_log2_positive",
				density_key: "coverage_density_positive"
			}
		],
		lines: [
			{
				y: 0,
				style: "rgba(50, 50, 200, 0.5)"
			}
		]
	},
	{
		height: 50,
		label: "BAF Means",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_mean",
				density_key: "baf_density"
			},
			{
				key: "baf_mid_mean",
				density_key: "baf_mid_density"
			},
			{
				key: "baf_bot_mean",
				density_key: "baf_bot_density"
			},
			{
				key: "baf_top_mean",
				density_key: "baf_top_density"
			}
		]
	},
	{
		height: 50,
		label: "BAF Deviation Mean",
		view: {
			top_base: 0.55,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_dev_mean",
				density_key: "baf_density"
			}
		]
	},
	{
		height: 50,
		label: "BAF Total Density",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_density",
				func: value => value / (value + 0.5),
				fill: [0.05, 0.85, 0.05, 0.5]
			}
		]
	},
	{
		height: 50,
		label: "BAF Top Density",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_top_density",
				func: value => value / (value + 0.5),
				fill: [0.05, 0.85, 0.05, 0.5]
			}
		]
	},
	{
		height: 50,
		label: "BAF Middle Density",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_mid_density",
				func: value => value / (value + 0.5),
				fill: [0.05, 0.85, 0.05, 0.5]
			}
		]
	},
	{
		height: 50,
		label: "BAF Bottom Density",
		view: {
			top_base: 1.05,
			bot_base: -0.05
		},
		data: [
			{
				key: "baf_bot_density",
				func: value => value / (value + 0.5),
				fill: [0.05, 0.85, 0.05, 0.5]
			}
		]
	}
];

function rgb_to_hex(color) {
	return "#" + (1 << 24 | color[0] << 16 | color[1] << 8 | color[2]).toString(16).slice(1);
}

function hex_to_rgb(color) {
	return [
		parseInt(color.slice(1, 3), 16),
		parseInt(color.slice(3, 5), 16),
		parseInt(color.slice(5, 7), 16),
		1.0
	];
}

function is_parent_recursive(child, parent) {
	while (child) {
		if (child === parent)
			return true;
		child = child.parentNode;
	}
	return false;
}

window.onload = function () {
	bin_sizes = [base_bin_size];
	while (bin_sizes[bin_sizes.length - 1] < max_bin_size) {
		bin_sizes.push(bin_sizes[bin_sizes.length - 1] * 2);
	}

	let context_menu_element = document.createElement("div");
	context_menu_element.classList.add("context_menu");
	context_menu_element.style.display = "none";
	document.body.appendChild(context_menu_element);

	canvas = document.createElement("canvas");
	canvas_wrapper = document.createElement("div");
	canvas_wrapper.classList.add("canvas_wrapper");
	canvas_wrapper.appendChild(canvas);
	document.body.appendChild(canvas_wrapper);

	document.addEventListener("pointerdown", (ev) => {
		if (is_parent_recursive(ev.target, context_menu_element) == false) {
			context_menu_element.style.display = "none";
		}
	});

	let views_element = document.getElementById("views");
	for (let clip of clips) {
		clip.wrapper = document.createElement("div");
		clip.wrapper.className = "clip_wrapper";
		views_element.appendChild(clip.wrapper);

		clip.rect = {};

		if (clip.type == null || clip.type == "annotation") {
			clip.render_area = document.createElement("div");
			clip.render_area.style.height = clip.height + "px";
			clip.render_area.classList.add("canvas_cursor_text");
			clip.wrapper.appendChild(clip.render_area);

			clip.render_area.oncontextmenu = (ev) => {
				let pos = (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real + focus;
				if (calls) {
					let call_chr = calls.chromosomes[chr.name];
					if (call_chr) {
						let filtered_calls = call_chr.records.filter(e => e.pos <= pos && e.info["END"] >= pos);
						if (filtered_calls.length > 0) {
							context_menu_element.style.display = "";
							context_menu_element.style.left = ev.clientX + "px";
							context_menu_element.style.top = ev.clientY + "px";
							context_menu_element.innerHTML = "";
							for (let call of filtered_calls) {
								let p = document.createElement("p");
								p.innerText = call.id;
								context_menu_element.appendChild(p);
							}
							return false;
						}
					}
				}
			};

			clip.render_area.onpointerdown = (ev) => {
				if (ev.which == 1) {
					zoom_area_start = (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real + focus;
					zoom_area_end = zoom_area_start;
					view_mouse_x = ev.clientX;
					clip.render_area.setPointerCapture(ev.pointerId);
					clip.render_area.onpointerup = (ev) => {
						if (ev.which != 1)
							return;
						zoom_area_end = (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real + focus;
						if (Math.abs(zoom_area_end - zoom_area_start) / screen_to_real >= min_zoom_area_width) {
							perform_zoom(zoom_area_start, zoom_area_end);
						}
						zoom_area_start = null;
						zoom_area_end = null;
						clip.render_area.releasePointerCapture(ev.pointerId);
						clip.render_area.onpointerup = null;
					};
				}
			};

			clip.wrapper.onpointerdown = (ev) => {
				if (ev.which == 2) {
					clip.grab_position = (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real + focus;
					clip.render_area.classList.add("grabbing");
					clip.render_area.setPointerCapture(ev.pointerId);
					clip.render_area.onpointerup = (ev) => {
						if (ev.which != 2)
							return;
						clip.render_area.classList.remove("grabbing");
						clip.render_area.releasePointerCapture(ev.pointerId);
						clip.grab_position = null;
						clip.render_area.onpointerup = null;

						// scuffed
						smooth_zoom_target_focus = focus;
						smooth_zoom_target_screen_to_real = screen_to_real;
					};
					ev.preventDefault();
				}
			};

			function pointermove(ev) {
				let position = (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real + focus;
				if (zoom_area_start)
					zoom_area_end = position;
				view_mouse_x = ev.clientX;
				if (clip.grab_position != null) {
					let last_focus = focus;
					focus = clip.grab_position - (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real;
					position = (ev.clientX - document.documentElement.clientWidth * 0.5) * screen_to_real + focus;
					if (last_focus != focus)
						render_out_of_date = true;
				}
				if (clip.type == "annotation") {
					if (clip.chromosomes && chr.name in clip.chromosomes) {
						let annotations = clip.chromosomes[chr.name];
						if (annotations) {
							let hovered_indices = [];
							for (let i = 0; i < annotations.count; ++i) {
								if (position >= annotations.start[i] && position <= annotations.end[i]) {
									hovered_indices.push(i);
								}
							}
							if (hovered_indices.length > 0) {
								clip.tooltip.style.display = "";
								clip.tooltip.innerHTML = "";
								clip.tooltip.style.left = ev.clientX - 50 + "px";
								clip.tooltip.style.bottom = "calc(100% - 5px)";

								let gene_names = {};

								for (const i of hovered_indices) {
									if (annotations.gene_name[i] < clip.lookup.gene_names.length) {
										gene_names[annotations.gene_id[i]] = clip.lookup.gene_names[annotations.gene_name[i]];
									}
								}

								for (const [gene_id, gene_name] of Object.entries(gene_names)) {
									let tr = document.createElement("tr");
									
									let td = document.createElement("td");
									td.innerHTML = gene_name;
									tr.appendChild(td);

									td = document.createElement("td");
									let a = document.createElement("a");
									let s = "00000000000" + gene_id;
									s = "ENSG" + s.substr(-11);
									a.innerHTML = s;
									a.href = "https://grch37.ensembl.org/Homo_sapiens/Gene/Summary?db=core;g=" + s;
									a.target = "_blank";
									td.appendChild(a);
									tr.appendChild(td);

									clip.tooltip.appendChild(tr);
								}
							} else {
								clip.tooltip.style.display = "none";
							}
						}
					}
				}
			}
			clip.render_area.addEventListener("pointermove", pointermove);
			
			if (clip.type == "annotation") {
				clip.chromosomes = {};

				fetch(clip.source + "lookup.json")
				.then(response => response.json())
				.then(data => {
					clip.lookup = data;
					clip.render_out_of_date = true;
				});

				clip.tooltip = document.createElement("table");
				clip.tooltip.classList.add("annotation_tooltip");
				clip.tooltip.style.display = "none";
				clip.wrapper.appendChild(clip.tooltip);

				clip.wrapper.onpointerleave = (ev) => {
					clip.tooltip.style.display = "none";
				};
			}
		}
		if (clip.type == "overview") {
			if (clip.data) {
				clip.overview_wrapper = document.createElement("div");
				clip.overview_wrapper.style.marginRight = "50px";
				clip.wrapper.appendChild(clip.overview_wrapper);
	
				clip.subclips = [];
				let total_size = chromosome_sizes.reduce((a, e) => a + e);
				for (let i = 0; i < chromosome_sizes.length; ++i) {
					let subclip = {};
					subclip.rect = {};
					clip.subclips.push(subclip);

					let a = document.createElement("a");
					a.href = "#" + chromosome_names[i];
					a.classList.add("overview_link");
					a.style.width = chromosome_sizes[i] / total_size * 100 + "%";
					clip.overview_wrapper.appendChild(a);
	
					let render_area = document.createElement("div");
					render_area.style.height = clip.height + "px";
					subclip.render_area = render_area;
					a.appendChild(render_area);

					let overlay = document.createElement("div");
					overlay.classList.add("overlay");
					a.appendChild(overlay);
	
					let highlight_overlay = document.createElement("div");
					highlight_overlay.classList.add("overlay");
					highlight_overlay.classList.add("overview_highlight_overlay");
					overlay.appendChild(highlight_overlay);
	
					let zoom_overlay = document.createElement("div");
					zoom_overlay.classList.add("overlay");
					zoom_overlay.classList.add("overview_zoom_overlay");
					//zoom_overlay.style.display = "none";
					overlay.appendChild(zoom_overlay);

					let chr_name_element = document.createElement("div");
					chr_name_element.innerText = chromosome_names[i];
					overlay.appendChild(chr_name_element);
				}
			} else {
				clip.render_area = document.createElement("div");
				clip.render_area.style.height = clip.height + "px";
				clip.wrapper.appendChild(clip.render_area);
			}
		}

		clip.resizer = document.createElement("div");
		clip.resizer.classList.add("clip_resizer");
		clip.resizer.onpointerdown = (ev) => {
			if (ev.which == 1) {
				clip.resizer.setPointerCapture(ev.pointerId);
				clip.resizer.onpointermove = (e) => {
					let height = Math.round(e.clientY - clip.render_area.getBoundingClientRect().top);
					height = Math.max(0, height);
					height += "px";
					if (clip.render_area.style.height != height) {
						clip.render_area.style.height = height;
						render_out_of_date = true;
					}
				};
				return false;
			}
		};
		clip.resizer.onpointerup = (ev) => {
			if (ev.which == 1) {
				clip.resizer.releasePointerCapture(ev.pointerId);
				clip.resizer.onpointermove = null;
			}
		};
		clip.wrapper.appendChild(clip.resizer);

		clip.grabber = document.createElement("img");
		clip.grabber.src = "../../../../cnv_visualizer/data_viewer/handle2.svg";
		clip.grabber.classList.add("clip_grabber");
		clip.grabber.onpointerdown = (ev) => {
			clip.grabber.style.cursor = "grabbing";
			let grabber_rect = clip.grabber.getBoundingClientRect();
			let grab_offset_y = (grabber_rect.top + grabber_rect.height * 0.5) - ev.clientY;
			clip.grabber.setPointerCapture(ev.pointerId);
			clip.grabber.onpointermove = (e) => {
				let wrapper_rect = clip.wrapper.getBoundingClientRect();
				let y_offset = wrapper_rect.height * 0.5;
				let grabber_y = e.clientY + grab_offset_y;

				let parent = clip.wrapper.parentNode;
				let wrappers = Array.from(parent.children);

				let closest_diff = grabber_y - (wrappers[wrappers.length - 1].getBoundingClientRect().bottom + y_offset - wrapper_rect.height);
				let closest_index = wrappers.length;

				for (let i = 0; i < wrappers.length; ++i) {
					let rect = wrappers[i].getBoundingClientRect();
					let y = rect.top + y_offset;
					let diff = grabber_y - y;
					if (Math.abs(diff) < Math.abs(closest_diff)) {
						closest_diff = diff;
						closest_index = i;
					}
					if (wrappers[i] === clip.wrapper) {
						y_offset -= rect.height;
					}
				}
				if (wrappers[closest_index] !== clip.wrapper) {
					for (let i = 0; i < closest_index; ++i) {
						if (wrappers[i] !== clip.wrapper)
							parent.appendChild(wrappers[i]);
					}
					parent.appendChild(clip.wrapper);
					for (let i = closest_index; i < wrappers.length; ++i) {
						if (wrappers[i] !== clip.wrapper)
							parent.appendChild(wrappers[i]);
					}
					clip.grabber.setPointerCapture(e.pointerId);
					render_out_of_date = true;
				}
				clip.grabber.style.transform = `translateY(calc(${closest_diff}px - 50%))`;
			};
			return false;
		};
		clip.grabber.onpointerup = (ev) => {
			clip.grabber.style.cursor = "";
			clip.grabber.style.transform = "";
			clip.grabber.releasePointerCapture(ev.pointerId);
			clip.grabber.onpointermove = null;
		};
		clip.wrapper.appendChild(clip.grabber);
		
		let overlay = document.createElement("div");
		overlay.className = "clip_overlay";
		clip.wrapper.appendChild(overlay);

		let label = document.createElement("a");
		label.innerHTML = clip.label ?? "";
		label.className = "label";
		overlay.appendChild(label);
		
		let cogwheel = document.createElement("img");
		cogwheel.className = "cogwheel";
		cogwheel.src = "../../../../cnv_visualizer/data_viewer/cogwheel.svg";
		cogwheel.alt = "Options";
		overlay.appendChild(cogwheel);

		let options_panel = document.createElement("div");
		options_panel.className = "options_panel";
		options_panel.style.display = "none";
		overlay.appendChild(options_panel);

		cogwheel.onclick = () => {
			options_panel.style.display = options_panel.style.display == "none" ? "block" : "none";
		};

		if (clip.data) {
			for (let data of clip.data) {
				if (data.colors) {
					for (let i = 0; i < data.colors.length; ++i) {
						let label = document.createElement("label");
						label.innerText = data.labels[i];
						options_panel.appendChild(label);

						let input = document.createElement("input");
						input.type = "color";
						input.value = rgb_to_hex(data.colors[i].map(e => Math.floor(e * 255)));
						input.oninput = (ev) => {
							data.colors[i] = hex_to_rgb(ev.target.value).map(e => e / 255);
							clip.render_out_of_date = true;
						};
						options_panel.appendChild(input);
					}
				}
				if (data.amplification) {
					let slider = document.createElement("input");
					slider.type = "range";
					slider.min = "1";
					slider.max = "10";
					slider.step = "0.01";
					slider.value = data.amplification;
					slider.oninput = (ev) => {
						data.amplification = parseFloat(ev.target.value);
						clip.render_out_of_date = true;
					};
					options_panel.appendChild(slider);
				}
				if (data.point_mod) {
					let toggle = document.createElement("input");
					toggle.type = "checkbox";
					let inital_value = data.point_mod;
					toggle.checked = true;
					toggle.oninput = (ev) => {
						data.point_mod = ev.target.checked ? inital_value : null;
						clip.render_out_of_date = true;
					};
					options_panel.appendChild(toggle);
				}
			}
		}
	}

	hash_change_handler();

	fetch("calls.json")
	.then(response => response.json())
	.then(data => {
		calls = data;
		render_out_of_date = true;
	});

	window.requestAnimationFrame(on_animation_frame);
};
