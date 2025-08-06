#include <minec_client.h>

#ifdef MINEC_CLIENT_DEBUG_LOG

static void DebugCallback(
	GLenum source, 
	GLenum type, 
	GLuint id, 
	GLenum severity,
	GLsizei length, 
	const GLchar* message, 
	const void* userParam
)
{
	struct minec_client* client = (struct minec_client*)userParam;

	minec_client_log_debug(client, "--OpenGL-Debug-Message-- Source: %u | Type : %u | ID : %u | Severity : %u | Message : %s", source, type, id, severity, message);
}

uint32_t _debug_messenger_init(struct minec_client* client)
{
	OPENGL.func.glEnable(GL_DEBUG_OUTPUT);
	OPENGL.func.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	OPENGL.func.glDebugMessageCallback(DebugCallback, (const void*)client);

	OPENGL.func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, NULL, GL_TRUE);
	OPENGL.func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, NULL, GL_TRUE);
	OPENGL.func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, NULL, GL_FALSE);
	OPENGL.func.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

	if (opengl_error_get_log(client, "Initializing debug messenger")) return MINEC_CLIENT_ERROR;
	return MINEC_CLIENT_SUCCESS;
}

#endif