
#include "TextureImportDialog.h"
#include "Resources/Texture.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>

REGISTER_IMPORT_DIALOG(CTexture, CTextureImportDialog);

class CTextureImportWidget : public CFramelessDialog
{
public:
	CTextureImportWidget(FString fileName, QWidget* parent = nullptr) : CFramelessDialog(parent)
	{
		QFrame* frame = new QFrame(this);
		QVBoxLayout* layout = new QVBoxLayout(frame);

		setCentralWidget(frame);
		setTitle(QString("Import ") + fileName.c_str());

		QHBoxLayout* l1 = new QHBoxLayout();

		QLabel* formatLabl = new QLabel("Format: ", frame);
		QComboBox* formatEdit = new QComboBox(this);
		formatEdit->addItem("R 8bits");
		formatEdit->addItem("RG 16bits");
		formatEdit->addItem("RGB 24bits");
		formatEdit->addItem("RGBA 32bits");
		formatEdit->addItem("RGBA float (HDR)");
		formatEdit->addItem("Compressed (DXT1)");
		formatEdit->addItem("Normal Map (DXT5)");
		formatEdit->addItem("Auto Select");
		formatEdit->addItem("Auto Select (Compressed)");

		formatEdit->setCurrentIndex(THTX_FORMAT_AUTO_COMPRESSED);

		l1->addWidget(formatLabl);
		l1->addWidget(formatEdit);
		layout->addLayout(l1);

		QHBoxLayout* l2 = new QHBoxLayout();

		QLabel* mimmapLabl = new QLabel("MipMaps: ", frame);
		QSpinBox* mimmapEdit = new QSpinBox(this);
		mimmapEdit->setMinimum(1);
		mimmapEdit->setMaximum(10);
		mimmapEdit->setValue(6);

		l2->addWidget(mimmapLabl);
		l2->addWidget(mimmapEdit);
		layout->addLayout(l2);

		QHBoxLayout* l3 = new QHBoxLayout();

		QPushButton* btnImport = new QPushButton("Import", frame);
		btnImport->setProperty("type", QVariant("primary"));
		QPushButton* btnImportAll = new QPushButton("Import All", frame);
		QPushButton* btnCancel = new QPushButton("Cancel", frame);

		l3->addWidget(btnImport);
		l3->addWidget(btnImportAll);
		l3->addWidget(btnCancel);

		layout->addLayout(l3);

		connect(btnImport, &QPushButton::clicked, this, [=]() { settings.format = (ETextureAssetFormat)formatEdit->currentIndex(); settings.numMipMaps = mimmapEdit->value(); done(true); });
		connect(btnImportAll, &QPushButton::clicked, this, [=]() { settings.format = (ETextureAssetFormat)formatEdit->currentIndex(); settings.numMipMaps = mimmapEdit->value(); done(2); });
		connect(btnCancel, &QPushButton::clicked, this, [=]() { done(false); });
	}
	
	FTextureImportSettings settings;
};

CTextureImportDialog::CTextureImportDialog(QWidget* p /*= nullptr*/) : parent(p)
{
}

void CTextureImportDialog::Exec(const QStringList& f, const WString& out, const WString& m)
{
	path = out;
	mod = m;

	QStringList files = f;
	FTextureImportSettings lastSettings;

	bool bImportAll = false;
	for (auto file : files)
	{
		FString fileName = file.toStdString();
		if (auto i = fileName.FindLastOf("\\/"); i != -1)
			fileName.Erase(fileName.begin(), fileName.begin() + i + 1);

		if (bImportAll)
		{
			ImportTexture(file, lastSettings);
			continue;
		}

		CTextureImportWidget dialog(fileName, parent);
		int r = dialog.exec();
		if (r == 0)
			break;
		bImportAll = (r == 2);

		lastSettings = dialog.settings;
		ImportTexture(file, lastSettings);
	}
}

void CTextureImportDialog::ImportTexture(const QString& file, const FTextureImportSettings& settings)
{
	WString fileName = file.toStdWString();
	if (auto i = fileName.FindLastOf(L"\\/"); i != -1)
		fileName.Erase(fileName.begin(), fileName.begin() + i + 1);
	if (auto i = fileName.FindLastOf('.'); i != -1)
		fileName.Erase(fileName.begin() + i, fileName.end());

	TObjectPtr<CTexture> t = CResourceManager::CreateResource<CTexture>(path + L"\\" + fileName + ToWString(((FAssetClass*)CTexture::StaticClass())->GetExtension()), mod);
	if (!t->Import(file.toStdWString(), settings))
		t->File()->Mod()->DeleteFile(t->File()->Path());
}
