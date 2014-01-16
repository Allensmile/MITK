/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <itkAddArtifactsToDwiImageFilter.h>
#include <mitkFiberfoxParameters.h>
#include <mitkFiberBundleXReader.h>
#include <mitkStickModel.h>
#include <mitkTensorModel.h>
#include <mitkBallModel.h>
#include <mitkDotModel.h>
#include <mitkAstroStickModel.h>
#include <mitkDiffusionImage.h>
#include <mitkNrrdDiffusionImageWriter.h>
#include <itkTestingComparisonImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <mitkRicianNoiseModel.h>
#include <mitkChiSquareNoiseModel.h>
#include <mitkTestFixture.h>

/**Documentation
 *
 */
class mitkFiberfoxAddArtifactsToDwiTestSuite : public mitk::TestFixture
{

    CPPUNIT_TEST_SUITE(mitkFiberfoxAddArtifactsToDwiTestSuite);
    MITK_TEST(Spikes);
    MITK_TEST(GibbsRinging);
    MITK_TEST(Ghost);
    MITK_TEST(Aliasing);
    MITK_TEST(Eddy);
    MITK_TEST(RicianNoise);
    MITK_TEST(ChiSquareNoise);
    MITK_TEST(Distortions);
    CPPUNIT_TEST_SUITE_END();

private:

    mitk::DiffusionImage<short>::Pointer m_InputDwi;
    FiberfoxParameters m_Parameters;

public:

    void setUp()
    {
        RegisterDiffusionCoreObjectFactory();

        // reference files
        m_InputDwi = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(GetTestDataFilePath("DiffusionImaging/Fiberfox/StickBall_RELAX.dwi"))->GetData());

        // parameter setup
        m_Parameters = FiberfoxParameters();
        m_Parameters.m_ImageRegion = m_InputDwi->GetVectorImage()->GetLargestPossibleRegion();
        m_Parameters.m_ImageSpacing = m_InputDwi->GetVectorImage()->GetSpacing();
        m_Parameters.m_ImageOrigin = m_InputDwi->GetVectorImage()->GetOrigin();
        m_Parameters.m_ImageDirection = m_InputDwi->GetVectorImage()->GetDirection();
        m_Parameters.m_Bvalue = m_InputDwi->GetB_Value();
        mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer dirs = m_InputDwi->GetDirections();
        m_Parameters.m_NumGradients = 0;
        for (unsigned int i=0; i<dirs->Size(); i++)
        {
            DiffusionSignalModel<double>::GradientType g;
            g[0] = dirs->at(i)[0];
            g[1] = dirs->at(i)[1];
            g[2] = dirs->at(i)[2];
            m_Parameters.m_GradientDirections.push_back(g);
            if (dirs->at(i).magnitude()>0.0001)
                m_Parameters.m_NumGradients++;
        }
    }

    bool CompareDwi(itk::VectorImage< short, 3 >* dwi1, itk::VectorImage< short, 3 >* dwi2)
    {
        typedef itk::VectorImage< short, 3 > DwiImageType;
        try{
            itk::ImageRegionIterator< DwiImageType > it1(dwi1, dwi1->GetLargestPossibleRegion());
            itk::ImageRegionIterator< DwiImageType > it2(dwi2, dwi2->GetLargestPossibleRegion());
            while(!it1.IsAtEnd())
            {
                if (it1.Get()!=it2.Get())
                    return false;
                ++it1;
                ++it2;
            }
        }
        catch(...)
        {
            return false;
        }
        return true;
    }

    void StartSimulation(string testFileName)
    {
        mitk::DiffusionImage<short>::Pointer refImage = NULL;
        if (!testFileName.empty())
            CPPUNIT_ASSERT(refImage = dynamic_cast<mitk::DiffusionImage<short>*>(mitk::IOUtil::LoadDataNode(testFileName)->GetData()));

        itk::AddArtifactsToDwiImageFilter< short >::Pointer artifactsToDwiFilter = itk::AddArtifactsToDwiImageFilter< short >::New();
        artifactsToDwiFilter->SetUseConstantRandSeed(true);
        artifactsToDwiFilter->SetInput(m_InputDwi->GetVectorImage());
        artifactsToDwiFilter->SettLine(m_Parameters.m_tLine);
        artifactsToDwiFilter->SetkOffset(m_Parameters.m_KspaceLineOffset);
        artifactsToDwiFilter->SetNoiseModel(m_Parameters.m_NoiseModelShort);
        artifactsToDwiFilter->SetGradientList(m_Parameters.m_GradientDirections);
        artifactsToDwiFilter->SetTE(m_Parameters.m_tEcho);
        artifactsToDwiFilter->SetSimulateEddyCurrents(m_Parameters.m_DoSimulateEddyCurrents);
        artifactsToDwiFilter->SetEddyGradientStrength(m_Parameters.m_EddyStrength);
        artifactsToDwiFilter->SetAddGibbsRinging(m_Parameters.m_AddGibbsRinging);
        artifactsToDwiFilter->SetFrequencyMap(m_Parameters.m_FrequencyMap);
        artifactsToDwiFilter->SetSpikeAmplitude(m_Parameters.m_SpikeAmplitude);
        artifactsToDwiFilter->SetSpikes(m_Parameters.m_Spikes);
        artifactsToDwiFilter->SetWrap(m_Parameters.m_Wrap);
        CPPUNIT_ASSERT_NO_THROW(artifactsToDwiFilter->Update());

        mitk::DiffusionImage<short>::Pointer testImage = mitk::DiffusionImage<short>::New();
        testImage->SetVectorImage( artifactsToDwiFilter->GetOutput() );
        testImage->SetB_Value(m_Parameters.m_Bvalue);
        testImage->SetDirections(m_Parameters.m_GradientDirections);
        testImage->InitializeFromVectorImage();

        if (refImage.IsNotNull())
        {
            CPPUNIT_ASSERT_MESSAGE(testFileName, CompareDwi(testImage->GetVectorImage(), refImage->GetVectorImage()));
        }
        else
        {
            NrrdDiffusionImageWriter<short>::Pointer writer = NrrdDiffusionImageWriter<short>::New();
            writer->SetFileName("/local/distortions2.dwi");
            writer->SetInput(testImage);
            writer->Update();
        }
    }

    void Spikes()
    {
        m_Parameters.m_Spikes = 5;
        m_Parameters.m_SpikeAmplitude = 1;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/spikes2.dwi") );
    }

    void GibbsRinging()
    {
        m_Parameters.m_AddGibbsRinging = true;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/gibbsringing2.dwi") );
    }

    void Ghost()
    {
        m_Parameters.m_KspaceLineOffset = 0.25;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/ghost2.dwi") );
    }

    void Aliasing()
    {
        m_Parameters.m_Wrap = 0.4;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/aliasing2.dwi") );
    }

    void Eddy()
    {
        m_Parameters.m_DoSimulateEddyCurrents = true;
        m_Parameters.m_EddyStrength = 0.05;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/eddy2.dwi") );
    }

    void RicianNoise()
    {
        mitk::RicianNoiseModel<double>* ricianNoiseModel = new mitk::RicianNoiseModel<double>();
        ricianNoiseModel->SetNoiseVariance(1000000);
        ricianNoiseModel->SetSeed(0);
        m_Parameters.m_NoiseModel = ricianNoiseModel;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/riciannoise2.dwi") );
        delete m_Parameters.m_NoiseModel;
    }

    void ChiSquareNoise()
    {
        mitk::ChiSquareNoiseModel<double>* chiSquareNoiseModel = new mitk::ChiSquareNoiseModel<double>();
        chiSquareNoiseModel->SetDOF(500000);
        chiSquareNoiseModel->SetSeed(0);
        m_Parameters.m_NoiseModel = chiSquareNoiseModel;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/chisquarenoise2.dwi") );
        delete m_Parameters.m_NoiseModel;
    }

    void Distortions()
    {
        mitk::Image::Pointer mitkFMap = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode( GetTestDataFilePath("DiffusionImaging/Fiberfox/Fieldmap.nrrd") )->GetData());
        typedef itk::Image<double, 3> ItkDoubleImgType;
        ItkDoubleImgType::Pointer fMap = ItkDoubleImgType::New();
        mitk::CastToItkImage<ItkDoubleImgType>(mitkFMap, fMap);
        m_Parameters.m_FrequencyMap = fMap;
        StartSimulation( GetTestDataFilePath("DiffusionImaging/Fiberfox/distortions2.dwi") );
    }
};

MITK_TEST_SUITE_REGISTRATION(mitkFiberfoxAddArtifactsToDwi)
