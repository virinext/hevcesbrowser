#ifndef SYNTAX_VIEWER_H_
#define SYNTAX_VIEWER_H_



#include <QTreeWidget>

#include <Hevc.h>
#include <HevcParser.h>

#include <map>
#include <memory>

#include "types.h"

class SyntaxViewer: public QTreeWidget
{
  Q_OBJECT

  public:
    SyntaxViewer(QWidget *pwgt = NULL);

    void updateItemsState();

  public slots:
    void onNalUChanged(std::shared_ptr<HEVC::NALUnit>, ParserInfo info);

    void setParameretsSets(const VPSMap &vpsMap,
                      const SPSMap &spsMap,
                      const PPSMap &ppsMap);

  private slots:
    void onItemCollapsed(QTreeWidgetItem * item);
    void onItemExpanded(QTreeWidgetItem * item);

private:
    void createVPS(std::shared_ptr<HEVC::VPS> pVPS);
    void createSPS(std::shared_ptr<HEVC::SPS> pSPS);
    void createPPS(std::shared_ptr<HEVC::PPS> pPPS);
    void createSlice(std::shared_ptr<HEVC::Slice> pSlice);
    void createAUD(std::shared_ptr<HEVC::AUD> pAUD);
    void createSEI(std::shared_ptr<HEVC::SEI> pSEI);

    void createProfileTierLevel(const HEVC::ProfileTierLevel &ptl, QTreeWidgetItem *pItem);
    void createVuiParameters(const HEVC::VuiParameters &vui, std::size_t maxNumSubLayersMinus1, QTreeWidgetItem *pItem);
    void createHrdParameters(const HEVC::HrdParameters &hrd, uint8_t commonInfPresentFlag, QTreeWidgetItem *pItem);
    void createSubLayerHrdParameters(const HEVC::SubLayerHrdParameters &slhrd, uint8_t sub_pic_hrd_params_present_flag, QTreeWidgetItem *pItem);
    void createShortTermRefPicSet(std::size_t stRpsIdx, const HEVC::ShortTermRefPicSet &rpset, std::size_t num_short_term_ref_pic_sets, const std::vector<HEVC::ShortTermRefPicSet> &refPicSets, QTreeWidgetItem *pItem);
    void createScalingListData(const HEVC::ScalingListData &scdata, QTreeWidgetItem *pItem);
    void createRefPicListModification(const HEVC::RefPicListModification &rplModif, QTreeWidgetItem *pItem);
    void createPredWeightTable(const HEVC::PredWeightTable &pwt, std::shared_ptr<HEVC::Slice> pSlice, QTreeWidgetItem *pItem);

    void createDecodedPictureHash(std::shared_ptr<HEVC::DecodedPictureHash> pDecPictHash, QTreeWidgetItem *pItem);
    void createUserDataUnregistered(std::shared_ptr<HEVC::UserDataUnregistered> pSei, QTreeWidgetItem *pItem);
    void createSceneInfo(std::shared_ptr<HEVC::SceneInfo> pSei, QTreeWidgetItem *pItem);
    void createFullFrameSnapshot(std::shared_ptr<HEVC::FullFrameSnapshot> pSei, QTreeWidgetItem *pItem);
    void createProgressiveRefinementSegmentStart(std::shared_ptr<HEVC::ProgressiveRefinementSegmentStart> pSeiPayload, QTreeWidgetItem *pItem);
    void createProgressiveRefinementSegmentEnd(std::shared_ptr<HEVC::ProgressiveRefinementSegmentEnd> pSeiPayload, QTreeWidgetItem *pItem);
    void createBufferingPeriod(std::shared_ptr<HEVC::BufferingPeriod> pSei, QTreeWidgetItem *pItem);
    void createPicTiming(std::shared_ptr<HEVC::PicTiming> pSei, QTreeWidgetItem *pItem);
    void createRecoveryPoint(std::shared_ptr<HEVC::RecoveryPoint> pSei, QTreeWidgetItem *pItem);
    void createToneMapping(std::shared_ptr<HEVC::ToneMapping> pSei, QTreeWidgetItem *pItem);
    void createFramePacking(std::shared_ptr<HEVC::FramePacking> pSei, QTreeWidgetItem *pItem);
    void createDisplayOrientation(std::shared_ptr<HEVC::DisplayOrientation> pSei, QTreeWidgetItem *pItem);
    void createSOPDescription(std::shared_ptr<HEVC::SOPDescription> pSei, QTreeWidgetItem *pItem);
    void createActiveParameterSets(std::shared_ptr<HEVC::ActiveParameterSets> pSeiPayload, QTreeWidgetItem *pItem);
    void createTemporalLevel0Index(std::shared_ptr<HEVC::TemporalLevel0Index> pSeiPayload, QTreeWidgetItem *pItem);
    void createRegionRefreshInfo(std::shared_ptr<HEVC::RegionRefreshInfo> pSeiPayload, QTreeWidgetItem *pItem);
    void createTimeCode(std::shared_ptr<HEVC::TimeCode> pSei, QTreeWidgetItem *pItem);
    void createMasteringDisplayInfo(std::shared_ptr<HEVC::MasteringDisplayInfo> pSei, QTreeWidgetItem *pItem);
    void createSegmRectFramePacking(std::shared_ptr<HEVC::SegmRectFramePacking> pSei, QTreeWidgetItem *pItem);
    void createKneeFunctionInfo(std::shared_ptr<HEVC::KneeFunctionInfo> pSei, QTreeWidgetItem *pItem);
    void createChromaResamplingFilterHint(std::shared_ptr<HEVC::ChromaResamplingFilterHint> pSeiPayload, QTreeWidgetItem *pItem);
    void createColourRemappingInfo(std::shared_ptr<HEVC::ColourRemappingInfo> pSeiPayload, QTreeWidgetItem *pItem);
    void createContentLightLevelInfo(std::shared_ptr<HEVC::ContentLightLevelInfo> pSeiPayload, QTreeWidgetItem *pItem);
    void createAlternativeTransferCharacteristics(std::shared_ptr<HEVC::AlternativeTransferCharacteristics> pSeiPayload, QTreeWidgetItem *pItem);

    std::map<uint32_t, std::shared_ptr<HEVC::VPS> >          m_vpsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::SPS> >          m_spsMap;
    std::map<uint32_t, std::shared_ptr<HEVC::PPS> >          m_ppsMap;

    class SyntaxViewerState
    {
    public:
      bool isActive(const QTreeWidgetItem *) const;
      void setActive(const QTreeWidgetItem *, bool isActive);

    private:
      QString name(const QTreeWidgetItem *) const;

      std::map<QString, bool>       m_state;
    };


    SyntaxViewerState     m_state;
};



#endif
