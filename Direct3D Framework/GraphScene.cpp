#include "pch.h"
#include <memory>
#include <utility>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include "GraphScene.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Model.h"
#include "Game.h"
#include "Common.h"
#include "SpriteString2D.h"

// �R���X�g���N�^
GraphScene::GraphScene(Game* game)
	:
	m_game(game),									// Game�N���X
	m_graphics(nullptr),				// DirectXGraphics�N���X
	m_device(nullptr),								// Device�N���X
	m_context(nullptr),								// DeviceContext�N���X
	m_keyboardState{},								// �L�[�{�[�h�X�e�[�g
	m_mouseCursorPosition{},					// �}�E�X�J�[�\���ʒu
	m_mouseState{},									// �}�E�X�X�e�[�g
	m_mouseStateTracker{},						// �}�E�X�X�e�[�g�g���b�J�[
	m_worldMatrix{},									// ���[���h
	m_viewMatrix{},									// �r���[
	m_projectionMatrix{},							// �v���W�F�N�V����
	m_cameraRotation{},							// �J������]
	m_cameraPosition{},							// �J�����ʒu
	m_cameraFocus{},								// �J�����t�H�[�J�X
	m_rotaionAngle(0.0f),							// �p�x
	m_distance(10.0f),								// �����_���王�_�܂ł̋���
	m_zoom(1.0f),										// �Y�[��
	m_fov(DirectX::XM_PI / 4.0f),				// �t�B�[���h�I�u�r���[
	m_nearPlane(0.1f),								// �j�A�N���b�v
	m_farPlane(0.0f),								// �t�@�[�N���b�v
	m_scale(1.0f),										// �X�P�[��
	m_angleA(0.0f),
	m_angleB(0.0f),
	m_vectorA(DirectX::SimpleMath::Vector2::Zero),
	m_vectorB(DirectX::SimpleMath::Vector2::Zero),
	m_result(0.0f)
{
	// DirectX Graphics�N���X�̃C���X�^���X���擾����
	m_graphics = Graphics::GetInstance();
	// �f�o�C�X���擾����
	m_device = Graphics::GetInstance()->GetDeviceResources()->GetD3DDevice();
	// �f�o�C�X�R���e�L�X�g���擾����
	m_context = Graphics::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

// �f�X�g���N�^
GraphScene::~GraphScene()
{
}

// ����������
void GraphScene::Initialize()
{
}

// �X�V����
void GraphScene::Update(const DX::StepTimer& timer)
{
	// �L�[�{�[�h�̏�Ԃ��擾����
	m_keyboardState = m_game->GetKeyboard()->GetState();
	// �L�[�{�[�h�̏�Ԃ��X�V����
	m_game->GetKeyboardTracker().Update(m_keyboardState);

	// �}�E�X�̏�Ԃ��擾����
	m_mouseState = m_game->GetMouse()->GetState();
	// �}�E�X�g���b�J�[���X�V����
	m_mouseStateTracker.Update(m_mouseState);

	// ���_�x�N�g�����擾����
	auto eyePosition = m_game->GetCamera()->GetEyePosition();
	// ���_�ƒ����_�̋������v�Z����
	m_distance = eyePosition.Length();

	// ���E�L�[�Ńx�N�g��A�̕�����ς���
	if (m_keyboardState.Left)
		m_angleA -= 1.0f;
	if (m_keyboardState.Right)
		m_angleA += 1.0f;
	// �㉺�L�[�Ńx�N�g��B�̕�����ς���
	if (m_keyboardState.Up)
		m_angleB -= 1.0f;
	if (m_keyboardState.Down)
		m_angleB += 1.0f;

	// ������ݒ肷��
	DirectX::SimpleMath::Vector2 directionA(0.0f, -1.0f);
	// ��]�s��𐶐�����
	DirectX::SimpleMath::Matrix rotationA = DirectX::SimpleMath::Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angleA));
	// �x�N�g��A�ɕ����Ƒ傫��(50)��ݒ肷��
	m_vectorA = DirectX::SimpleMath::Vector2::Transform(directionA, rotationA) * 50.0f;
	// ������ݒ肷��
	DirectX::SimpleMath::Vector2 directionB(0.0f, 1.0f);
	// ��]�s��𐶐�����
	DirectX::SimpleMath::Matrix rotationB = DirectX::SimpleMath::Matrix::CreateRotationZ(DirectX::XMConvertToRadians(m_angleB));
	// �x�N�g��B�ɕ����Ƒ傫��(50)��ݒ肷��
	m_vectorB = DirectX::SimpleMath::Vector2::Transform(directionB, rotationB) * 50.0f;
	// ���V�t�g�L�[�������������ꍇ
	if (m_keyboardState.LeftShift)
	{
		// ���ς̌v�Z���s��
		m_result = Dot2D(m_vectorA, m_vectorB);
	}
	else
	{
		// �O�ς̌v�Z���s��
		m_result = Cross2D(m_vectorA, m_vectorB);
	}

	// ���ʂ�����������
	DirectX::SimpleMath::Plane plane(0.0f, 1.0f, 0.0f, 0.0f);
	// �}�E�X�J�[�\���̃X�N���[���ʒu���擾����
	m_mouseCursorPosition = DirectX::SimpleMath::Vector2(roundf((float)m_mouseState.x), roundf((float)m_mouseState.y));
	// �J�������R���g���[������
	ControlCamera(timer);
}

// �`�悷��
void GraphScene::Render()
{
	const DirectX::XMVECTORF32 xaxis = { 100.f, 0.0f, 0.0f };
	const DirectX::XMVECTORF32 yaxis = { 0.0f, 0.0f, 100.f };

	// �O���b�h��`�悷��
	DrawGrid(xaxis, yaxis, DirectX::g_XMZero, 10, 10, DirectX::Colors::DarkGray);
	// �v���~�e�B�u�`����J�n����
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());

	// �x�N�g��A��`�悷��
	m_graphics->DrawVector(DirectX::SimpleMath::Vector2(0.0f, 0.0f), m_vectorA, DirectX::Colors::White);
	// ���F�̉~��`�悷��
	m_graphics->DrawCircle(m_vectorA, 10.0f, DirectX::Colors::White);

	// ���ς܂��͊O�ς̌v�Z���ʂ�0���傫���Ȃ����ꍇ
	if (m_result > 0)
	{
		// ���ς̏ꍇ�x�N�g��B�̓x�N�g��A�̑O���ɑ��݂���
		// �O�ς̏ꍇ�x�N�g��B�̓x�N�g��A�̉E���ɑ��݂���
		m_graphics->DrawVector(DirectX::SimpleMath::Vector2(0.0f, 0.0f), m_vectorB, DirectX::Colors::Red);
		// �ԐF�̉~��`�悷��
		m_graphics->DrawCircle(m_vectorB, 10.0f, DirectX::Colors::Red);
	}
	else
	{
		// ���ς̏ꍇ�x�N�g��B�̓x�N�g��A�̌���ɑ��݂���
		// �O�ς̏ꍇ�x�N�g��B�̓x�N�g��A�̍����ɑ��݂���
		m_graphics->DrawVector(DirectX::SimpleMath::Vector2(0.0f, 0.0f), m_vectorB, DirectX::Colors::Blue);
		// �F�̉~��`�悷��
		m_graphics->DrawCircle(m_vectorB, 10.0f, DirectX::Colors::Blue);
	}
	// �v���~�e�B�u�`����I������
	m_graphics->DrawPrimitiveEnd();
	// ����\������
	DrawInfo();
}

// �㏈�����s��
void GraphScene::Finalize()
{
}

// ����\������
void GraphScene::DrawInfo()
{
	wchar_t stringBuffer[128];
	// SpriteString2D��錾����
	SpriteString2D spriteString2D;

	// �J�����ʒu������������
	swprintf(stringBuffer, sizeof(stringBuffer) / sizeof(wchar_t), L"Camera position: (%6.1f, %6.1f, %6.1f)", 
	m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z);
	spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 0.0f));

	// ����/�O�ς�����������
	swprintf(stringBuffer, 	sizeof(stringBuffer) / sizeof(wchar_t), L"Dot/Cross Product: %6.1f", m_result);
	// ���ϊO�Ϗ���������������ƕ\��������W��ǉ�����
	spriteString2D.AddString(stringBuffer, DirectX::SimpleMath::Vector2(0.0f, 28.0f));
	// ���ׂĂ̏���`�悷��
	spriteString2D.Render();
}

// �A�[�N�{�[�����g�p���ăJ�������R���g���[������
void GraphScene::ControlCamera(const DX::StepTimer& timer)
{
	// �o�ߎ��Ԃ��擾����
	float elapsedTime = float(timer.GetElapsedSeconds());
	// �X�P�[���Q�C��
	constexpr float SCALE_GAIN = 0.001f;

	// �J�����ړ��s��
	DirectX::SimpleMath::Matrix im;
	m_viewMatrix.Invert(im);
	DirectX::SimpleMath::Vector3 move = DirectX::SimpleMath::Vector3::TransformNormal(move, im);

	// �}�E�X�̈ړ������΃}�E�X�ړ��ł���ꍇ
	if (m_game->GetMouse()->GetState().positionMode == DirectX::Mouse::MODE_RELATIVE)
	{
		DirectX::SimpleMath::Vector3 delta = DirectX::SimpleMath::Vector3(-float(m_mouseState.x), float(m_mouseState.y), 0.f) * m_distance;
		delta = DirectX::SimpleMath::Vector3::TransformNormal(delta, im);
		// �J�����t�H�[�J�X�ʒu���v�Z����
		m_cameraFocus += delta * elapsedTime;
	}
	// �}�E�X�̉E�{�^�����h���b�O���Ă���ꍇ
	else if (m_ballCamera.IsDragging())
	{
		// �}�E�X�̈ړ�
		m_ballCamera.OnMove(m_mouseState.x, m_mouseState.y);
		// �{�[���J�����̌��݂̃N�H�[�^�j�I�����擾����
		auto q = m_ballCamera.GetQuaternion();
		// �J������]�̋t�R�H�[�^�j�I�����v�Z����
		q.Inverse(m_cameraRotation);
	}
	else
	{
		// �}�E�X�t�H�C�[������]�������ꍇ�̃Y�[���l���v�Z����
		m_zoom = 1.0f + float(m_mouseState.scrollWheelValue) * SCALE_GAIN;
		// �Y�[���l�𒲐�����
		m_zoom = std::max(m_zoom, 0.01f);
		// �X�N���[���t�H�C�[���l�����Z�b�g����
		m_game->GetMouse()->ResetScrollWheelValue();
	}

	// �h���b�O���łȂ��ꍇ
	if (!m_ballCamera.IsDragging())
	{
		// �}�E�X�̉E�{�^�������������Ă���ꍇ
		if (m_mouseStateTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED)
		{
			// ���E��[Ctrl�L�[]�����������Ă���ꍇ
			if (m_keyboardState.LeftControl || m_keyboardState.RightControl)
			{
				// �{�[���J�������J�n����
				m_ballCamera.OnBegin(m_mouseState.x, m_mouseState.y);
			}
		}
	}
	// �}�E�X�̉E�{�^����������Ă���ꍇ
	else if (m_mouseStateTracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED)
	{
		// �{�[���J�������I������
		m_ballCamera.OnEnd();
	}
	// �J�����̌������X�V����
	auto direction = DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3::Backward, m_cameraRotation);
	// �J�����ʒu���v�Z����
	m_cameraPosition = m_cameraFocus + (m_distance * m_zoom) * direction;
	// �����x�N�g����ݒ肷��
	m_game->GetCamera()->SetEyePosition(m_cameraPosition);
}

// �O���b�h��`�悷��
void GraphScene::DrawGrid(
	const DirectX::FXMVECTOR& xAxis,
	const DirectX::FXMVECTOR& yAxis,
	const DirectX::FXMVECTOR& origin,
	const size_t& xdivs,
	const size_t& ydivs,
	const DirectX::GXMVECTOR& m_color
)
{
	// �p�t�H�[�}���X�J�n�C�x���g
	m_graphics->GetDeviceResources()->PIXBeginEvent(L"Draw Grid");
	// �v���~�e�B�u�`����J�n����
	m_graphics->DrawPrimitiveBegin(m_graphics->GetViewMatrix(), m_graphics->GetProjectionMatrix());

	for (size_t index = 0; index <= xdivs; ++index)
	{
		float percent = float(index) / float(xdivs);
		percent = (percent * 2.0f) - 1.0f;
		// �X�P�[�����v�Z����
		DirectX::XMVECTOR scale = DirectX::XMVectorScale(xAxis, percent);
		scale = DirectX::XMVectorAdd(scale, origin);
		// ���_1��ݒ肷��
		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, yAxis), m_color);
		// ���_2��ݒ肷��
		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, yAxis), m_color);
		// ������`�悷��
		m_graphics->GetPrimitiveBatch()->DrawLine(v1, v2);
	}

	for (size_t index = 0; index <= ydivs; index++)
	{
		float percent = float(index) / float(ydivs);
		percent = (percent * 2.0f) - 1.0f;
		// �X�P�[�����v�Z����
		DirectX::XMVECTOR scale = DirectX::XMVectorScale(yAxis, percent);
		scale = DirectX::XMVectorAdd(scale, origin);
		// ���_1��ݒ肷��
		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(scale, xAxis), m_color);
		// ���_2��ݒ肷��
		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(scale, xAxis), m_color);
		// ������`�悷��
		m_graphics->GetPrimitiveBatch()->DrawLine(v1, v2);
	}
	// �v���~�e�B�u�o�b�`���I������
	m_graphics->DrawPrimitiveEnd();
	// �p�t�H�[�}���X�I���C�x���g
	m_graphics->GetDeviceResources()->PIXEndEvent();
}

// �v���W�F�N�V�����𐶐�����
void GraphScene::CreateProjection()
{
	// �E�B���h�E�T�C�Y���擾����
	auto size = m_graphics->GetDeviceResources()->GetOutputSize();
	// �v���W�F�N�V�����𐶐�����
	m_projectionMatrix = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(m_fov, float(size.right) / float(size.bottom), m_nearPlane, m_farPlane);
}

// �A�[�N�{�[���̂��߂̃E�B���h�E�T�C�Y��ݒ肷��
void GraphScene::SetWindow(const int& width, const int& height)
{
	// �A�[�N�{�[���̃E�B���h�E�T�C�Y��ݒ肷��
	m_ballCamera.SetWindow(width, height);
}