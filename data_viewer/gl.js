class Shader {	
	constructor(gl, vert_source, frag_source, attributes, uniforms) {
		this.gl = gl;
		this.program = createShaderProgram(gl, vert_source, frag_source);
		for (const attribute of attributes)
			this[attribute] = gl.getAttribLocation(this.program, attribute);
		for (const uniform of uniforms)
			this[uniform] = gl.getUniformLocation(this.program, uniform);
	}
}

function createShaderProgram(gl, vert_source, frag_source) {
	const vert_shader = createShader(gl, gl.VERTEX_SHADER, vert_source);
	const frag_shader = createShader(gl, gl.FRAGMENT_SHADER, frag_source);

	const program = gl.createProgram();
	gl.attachShader(program, vert_shader);
	gl.attachShader(program, frag_shader);
	gl.linkProgram(program);

	if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
		alert(gl.getProgramInfoLog(program));
		return null;
	}

	return program;
}

function createShader(gl, type, source) {
	const shader = gl.createShader(type);

	gl.shaderSource(shader, source);

	gl.compileShader(shader);

	if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
		alert(gl.getShaderInfoLog(shader));
		gl.deleteShader(shader);
		return null;
	}

	return shader;
}

function createBuffer(gl, data) {
	let buffer = {};
	buffer.buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, buffer.buffer);

	gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);

	return buffer;
}

function createDynamicBuffer(gl, data) {
	let buffer = {};
	buffer.buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, buffer.buffer);

	gl.bufferData(gl.ARRAY_BUFFER, data, gl.DYNAMIC_DRAW);

	return buffer;
}

function updateDynamicBuffer(gl, buffer, data) {
	gl.bindBuffer(gl.ARRAY_BUFFER, buffer.buffer);
	gl.bufferData(gl.ARRAY_BUFFER, data, gl.DYNAMIC_DRAW);
}
