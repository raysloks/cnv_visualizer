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

function drawGraph(path, chunks, key, x_off, x_scale, y_off, y_scale, density_key, func) {

	let strokes = 0;
	let x;
	let y;

	for (let j = 0; j < chunks.length; ++j) {
		let data = chunks[j].data[key];
		if (!data)
			return;
		let density_data;
		if (density_key !== undefined)
			density_data = chunks[j].data[density_key];
		for (let i = 0; i < data.length; ++i) {
			let value = data[i];
			if (func)
				value = func(value);
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

	return x;
}

function drawPointGraph(path, chunks, key, x_off, x_scale, y_off, y_scale, density_key, func) {

	let x;
	let y;

	let point_width = Math.max(1, x_scale);

	for (let j = 0; j < chunks.length; ++j) {
		let data = chunks[j].data[key];
		if (!data)
			return;
		let density_data;
		if (density_key !== undefined)
			density_data = chunks[j].data[density_key];
		for (let i = 0; i < data.length; ++i) {
			let value = data[i];
			if (func)
				value = func(value);
			if (density_data)
				if (density_data[i] == 0)
					continue;
			x = x_off + i * x_scale;
			y = y_off + value * y_scale;
			path.moveTo(x, y);
			path.lineTo(x + point_width, y);
		}
		x_off += data.length * x_scale;
	}

	return x;
}

function drawRectangles(ctx, chunks, keys, colors, x_off, x_scale, y, height, amplification, density_key) {

	let x;

	for (let j = 0; j < chunks.length; ++j) {
		let data = keys.map(e => chunks[j].data[e]);
		if (data.some(e => !e))
			return;
		let density_data;
		if (density_key !== undefined)
			density_data = chunks[j].data[density_key];
		for (let i = 0; i < data[0].length; ++i) {
			let values = data.map(e => e[i]);
			if (density_data)
				if (density_data[i] != 0)
					values = values.map(e => e / density_data[i]);
				else
					continue;
			values = amplify_density_values(values, amplification);
			let color = colors.map((e, i) => e.map(e => e * values[i]))
				.reduce((acc, c) => acc.map((e, i) => e + c[i]), [0, 0, 0]);
			//color = xyz_to_rgb(color);
			color = rgb_to_srgb(color);
			color = color.map(e => Math.floor(e * 255));
			x = x_off + i * x_scale;
			ctx.fillStyle = `rgba(${color[0]}, ${color[1]}, ${color[2]}, 1.0)`;
			ctx.fillRect(x, y, x_scale + 1, height);
		}
		x_off += data[0].length * x_scale;
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

function render(render_bin_size, repaint) {

	{
		let overview_clips = clips.filter(e => e.type == "overview");
		for (let clip of overview_clips) {
			if (clip.canvases) {
				for (let i = 0; i < clip.canvases.length; ++i) {
					let canvas = clip.canvases[i];
					if (canvas.width != canvas.parentElement.clientWidth) {
			
						let max_chunk_index = Math.floor(chromosome_sizes[i] / (chunk_size * max_bin_size));
						let chunks = [];
						for (let j = 0; j <= max_chunk_index; ++j) {
							chunks.push(get_chunk(chromosome_names[i], max_bin_size, j));
						}
						
						if (chunks.some(e => e === null))
							continue;
		
						canvas.width = canvas.parentElement.clientWidth;
		
						let data = clip.data[0];
		
						let ctx = canvas.getContext("2d");
				
						ctx.setTransform(1, 0, 0, 1, 0, 0);
						ctx.clearRect(0, 0, canvas.width, canvas.height);
			
						drawRectangles(ctx, chunks, data.keys, data.colors, canvas.width * (0.5 - 0.5 / initial_view_margin), max_bin_size / chromosome_sizes[i] * canvas.width / initial_view_margin, 30, 20, data.amplification, data.density_key);
		
					}
				}
			}
			if (clip.canvas) {

				let current_chromosome_overview_a = overview_clips.find(e => e.canvases).canvases[chromosome_index].parentNode;
				let zoom_overlay = current_chromosome_overview_a.querySelector(".overview_zoom_overlay");
				
				let rect = zoom_overlay.getBoundingClientRect();
				
				let repaint_clip = repaint;

				if (clip.left !== rect.left || clip.right !== rect.right) {
					clip.left = rect.left;
					clip.right = rect.right;
					repaint_clip = true;
				}

				let canvas = clip.canvas;

				if (canvas.width != document.body.clientWidth || clip.height != canvas.height) {
					canvas.width = document.body.clientWidth;
					clip.height = canvas.height;
					repaint_clip = true;
				}

				if (repaint_clip) {
					let ctx = canvas.getContext("2d");

					ctx.setTransform(1, 0, 0, 1, 0, 0);
					ctx.clearRect(0, 0, canvas.width, canvas.height);

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

					ctx.lineWidth = 2;
					ctx.beginPath();
					ctx.moveTo(0, canvas.height);
					ctx.lineTo(rect.left, 0);
					ctx.moveTo(canvas.width, canvas.height);
					ctx.lineTo(rect.right, 0);
					ctx.stroke();
				}
			}
		}
	}

	let half_size = screen_to_real * document.body.clientWidth * 0.5;
	let [visible_chunks, bin_size] = getVisibleChunks(screen_to_real * resolution_modifier, focus - half_size, focus + half_size);

	let transcript_clips = clips.filter(e => e.type == "transcript");
	for (const clip of transcript_clips) {
	
		let canvas = clip.canvas;

		let repaint_clip = repaint;

		if (canvas.width != document.body.clientWidth) {
			canvas.width = document.body.clientWidth;
			repaint_clip = true;
		}

		if (clip.height != canvas.height) {
			clip.height = canvas.height;
			repaint_clip = true;
		}

		if (clip.render_out_of_date) {
			clip.render_out_of_date = false;
			repaint_clip = true;
		}

		if (repaint_clip === false) {
			continue;
		}

		let ctx = canvas.getContext("2d");

		ctx.setTransform(1, 0, 0, 1, 0, 0);
		ctx.clearRect(0, 0, canvas.width, canvas.height);
		
		if (chr.name in clip.chromosomes) {
			let transcripts = clip.chromosomes[chr.name];
			if (transcripts) {
				let colors = [
					"rgba(200, 0, 0, 0.25)",
					"rgba(100, 100, 0, 0.25)",
					"rgba(0, 200, 0, 0.25)",
					"rgba(0, 100, 100, 0.25)",
					"rgba(0, 0, 200, 0.25)",
					"rgba(100, 0, 100, 0.25)"
				];
				// this is probably really slow
				for (let i = 0; i < transcripts.count; ++i) {
					let screen_pos = (transcripts.start[i] - focus) / screen_to_real + canvas.width * 0.5;
					let screen_end = (transcripts.end[i] - focus) / screen_to_real + canvas.width * 0.5;
					if (screen_pos < canvas.width || screen_end > 0) {
						ctx.fillStyle = colors[i % colors.length];
						ctx.fillRect(screen_pos, 0, screen_end - screen_pos, canvas.height);
					}
				}
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

				let transcripts = {};
				transcripts.count = annotation_count;

				for (const data of clip_locked.lookup.data_sets) {
					offset += 7 - (offset - 1) % 8;
					switch (data.size) {
						case 1:
							transcripts[data.name] = new Uint8Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count;
							break;
						case 2:
							transcripts[data.name] = new Uint16Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 2;
							break;
						case 4:
							transcripts[data.name] = new Uint32Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 4;
							break;
						case 8:
							transcripts[data.name] = new Uint64Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 8;
							break;
					}
				}

				for (const data of clip_locked.lookup.data_prefixes) {
					offset += 7 - (offset - 1) % 8;
					switch (data.size) {
						case 1:
							transcripts[data.name] = new Uint8Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count;
							break;
						case 2:
							transcripts[data.name] = new Uint16Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 2;
							break;
						case 4:
							transcripts[data.name] = new Uint32Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 4;
							break;
						case 8:
							transcripts[data.name] = new Uint64Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 8;
							break;
					}
				}

				for (const data of clip_locked.lookup.data_values) {
					offset += 7 - (offset - 1) % 8;
					switch (data.size) {
						case 1:
							transcripts[data.name] = new Uint8Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count;
							break;
						case 2:
							transcripts[data.name] = new Uint16Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 2;
							break;
						case 4:
							transcripts[data.name] = new Uint32Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 4;
							break;
						case 8:
							transcripts[data.name] = new Uint64Array(arrayBuffer, offset, annotation_count);
							offset += annotation_count * 8;
							break;
					}
				}

				clip_locked.chromosomes[chr_name_locked] = transcripts;

				clip_locked.render_out_of_date = true;
			});
		}
	}

	let chromosome_clips = clips.filter(e => e.type == null);
	if (visible_chunks) {
		for (const clip of chromosome_clips) {
	
			let x_start = (visible_chunks[0].offset - focus) / screen_to_real + document.body.clientWidth * 0.5;
	
			let canvas = clip.canvas;
	
			let repaint_clip = repaint;
	
			if (canvas.width != document.body.clientWidth) {
				canvas.width = document.body.clientWidth;
				repaint_clip = true;
			}
	
			if (clip.height != canvas.height) {
				clip.height = canvas.height;
				repaint_clip = true;
			}
	
			if (clip.render_out_of_date) {
				clip.render_out_of_date = false;
				repaint_clip = true;
			}
	
			if (repaint_clip === false) {
				continue;
			}
	
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
	
			let top = (clip.view?.top_base ?? clip.height) + (clip.view?.top_scaling ?? 0) * clip.height;
			let bot = (clip.view?.bot_base ?? 0) + (clip.view?.bot_scaling ?? 0) * clip.height;
	
			let scale = canvas.height / (bot - top);
			let offset = -top * scale;
	
			if (clip.lines) {
				for (const line of clip.lines) {
					ctx.save();
					if (line.style)
						ctx.strokeStyle = line.style;
					ctx.beginPath();
					let y = line.y * scale + offset;
					ctx.moveTo(0, y);
					ctx.lineTo(canvas.width, y);
					ctx.stroke();
					ctx.restore();
				}
			}
	
			for (let data of clip.data) {
				if (data.key) {
					if (data.point_mod) {
						let [visible_chunks, bin_size] = getVisibleChunks(screen_to_real * resolution_modifier * data.point_mod, focus - half_size, focus + half_size);
						let x_start = (visible_chunks[0].offset - focus) / screen_to_real + document.body.clientWidth * 0.5;
						data.path = new Path2D();
						drawPointGraph(data.path, visible_chunks, data.key, x_start, bin_size / screen_to_real, offset, scale, data.density_key, data.func);
						ctx.stroke(data.path);
					} else {
						data.path = new Path2D();
						let x_end = drawGraph(data.path, visible_chunks, data.key, x_start, bin_size / screen_to_real, offset, scale, data.density_key, data.func);
						ctx.stroke(data.path);
						if (data.fill) {
							ctx.fillStyle = data.fill;
							data.path.lineTo(x_end, offset);
							data.path.lineTo(x_start, offset);
							ctx.fill(data.path);
						}
					}
				}
				if (data.keys)
					drawRectangles(ctx, visible_chunks, data.keys, data.colors, x_start, bin_size / screen_to_real, 0, canvas.height, data.amplification, data.density_key);
			}
	
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
	}
}
