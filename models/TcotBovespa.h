/**
 *
 *  TcotBovespa.h
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#pragma once
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/SqlBinder.h>
#include <drogon/orm/Mapper.h>
#include <drogon/orm/BaseBuilder.h>
#ifdef __cpp_impl_coroutine
#include <drogon/orm/CoroMapper.h>
#endif
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>
#include <json/json.h>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <tuple>
#include <stdint.h>
#include <iostream>

namespace drogon
{
namespace orm
{
class DbClient;
using DbClientPtr = std::shared_ptr<DbClient>;
}
}
namespace drogon_model
{
namespace testdb
{

class TcotBovespa
{
  public:
    struct Cols
    {
        static const std::string _dt_pregao;
        static const std::string _prz_termo;
        static const std::string _cd_codneg;
        static const std::string _cd_tpmerc;
        static const std::string _cd_codbdi;
        static const std::string _cd_codisin;
        static const std::string _nm_speci;
        static const std::string _prec_aber;
        static const std::string _prec_max;
        static const std::string _prec_min;
        static const std::string _prec_med;
        static const std::string _prec_fec;
        static const std::string _prec_exer;
        static const std::string _dt_datven;
        static const std::string _fat_cot;
        static const std::string _nr_dismes;
    };

    static const int primaryKeyNumber;
    static const std::string tableName;
    static const bool hasPrimaryKey;
    static const std::vector<std::string> primaryKeyName;
    using PrimaryKeyType = std::tuple<::trantor::Date,int32_t,std::string>;//dt_pregao,prz_termo,cd_codneg
    PrimaryKeyType getPrimaryKey() const;

    /**
     * @brief constructor
     * @param r One row of records in the SQL query result.
     * @param indexOffset Set the offset to -1 to access all columns by column names,
     * otherwise access all columns by offsets.
     * @note If the SQL is not a style of 'select * from table_name ...' (select all
     * columns by an asterisk), please set the offset to -1.
     */
    explicit TcotBovespa(const drogon::orm::Row &r, const ssize_t indexOffset = 0) noexcept;

    /**
     * @brief constructor
     * @param pJson The json object to construct a new instance.
     */
    explicit TcotBovespa(const Json::Value &pJson) noexcept(false);

    /**
     * @brief constructor
     * @param pJson The json object to construct a new instance.
     * @param pMasqueradingVector The aliases of table columns.
     */
    TcotBovespa(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false);

    TcotBovespa() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false);
    void updateByMasqueradedJson(const Json::Value &pJson,
                                 const std::vector<std::string> &pMasqueradingVector) noexcept(false);
    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForCreation(const Json::Value &,
                                                const std::vector<std::string> &pMasqueradingVector,
                                                    std::string &err);
    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForUpdate(const Json::Value &,
                                          const std::vector<std::string> &pMasqueradingVector,
                                          std::string &err);
    static bool validJsonOfField(size_t index,
                          const std::string &fieldName,
                          const Json::Value &pJson,
                          std::string &err,
                          bool isForCreation);

    /**  For column dt_pregao  */
    ///Get the value of the column dt_pregao, returns the default value if the column is null
    const ::trantor::Date &getValueOfDtPregao() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<::trantor::Date> &getDtPregao() const noexcept;
    ///Set the value of the column dt_pregao
    void setDtPregao(const ::trantor::Date &pDtPregao) noexcept;

    /**  For column prz_termo  */
    ///Get the value of the column prz_termo, returns the default value if the column is null
    const int32_t &getValueOfPrzTermo() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<int32_t> &getPrzTermo() const noexcept;
    ///Set the value of the column prz_termo
    void setPrzTermo(const int32_t &pPrzTermo) noexcept;

    /**  For column cd_codneg  */
    ///Get the value of the column cd_codneg, returns the default value if the column is null
    const std::string &getValueOfCdCodneg() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getCdCodneg() const noexcept;
    ///Set the value of the column cd_codneg
    void setCdCodneg(const std::string &pCdCodneg) noexcept;
    void setCdCodneg(std::string &&pCdCodneg) noexcept;

    /**  For column cd_tpmerc  */
    ///Get the value of the column cd_tpmerc, returns the default value if the column is null
    const int32_t &getValueOfCdTpmerc() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<int32_t> &getCdTpmerc() const noexcept;
    ///Set the value of the column cd_tpmerc
    void setCdTpmerc(const int32_t &pCdTpmerc) noexcept;

    /**  For column cd_codbdi  */
    ///Get the value of the column cd_codbdi, returns the default value if the column is null
    const int32_t &getValueOfCdCodbdi() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<int32_t> &getCdCodbdi() const noexcept;
    ///Set the value of the column cd_codbdi
    void setCdCodbdi(const int32_t &pCdCodbdi) noexcept;

    /**  For column cd_codisin  */
    ///Get the value of the column cd_codisin, returns the default value if the column is null
    const std::string &getValueOfCdCodisin() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getCdCodisin() const noexcept;
    ///Set the value of the column cd_codisin
    void setCdCodisin(const std::string &pCdCodisin) noexcept;
    void setCdCodisin(std::string &&pCdCodisin) noexcept;

    /**  For column nm_speci  */
    ///Get the value of the column nm_speci, returns the default value if the column is null
    const std::string &getValueOfNmSpeci() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getNmSpeci() const noexcept;
    ///Set the value of the column nm_speci
    void setNmSpeci(const std::string &pNmSpeci) noexcept;
    void setNmSpeci(std::string &&pNmSpeci) noexcept;
    void setNmSpeciToNull() noexcept;

    /**  For column prec_aber  */
    ///Get the value of the column prec_aber, returns the default value if the column is null
    const double &getValueOfPrecAber() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<double> &getPrecAber() const noexcept;
    ///Set the value of the column prec_aber
    void setPrecAber(const double &pPrecAber) noexcept;

    /**  For column prec_max  */
    ///Get the value of the column prec_max, returns the default value if the column is null
    const double &getValueOfPrecMax() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<double> &getPrecMax() const noexcept;
    ///Set the value of the column prec_max
    void setPrecMax(const double &pPrecMax) noexcept;

    /**  For column prec_min  */
    ///Get the value of the column prec_min, returns the default value if the column is null
    const double &getValueOfPrecMin() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<double> &getPrecMin() const noexcept;
    ///Set the value of the column prec_min
    void setPrecMin(const double &pPrecMin) noexcept;

    /**  For column prec_med  */
    ///Get the value of the column prec_med, returns the default value if the column is null
    const double &getValueOfPrecMed() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<double> &getPrecMed() const noexcept;
    ///Set the value of the column prec_med
    void setPrecMed(const double &pPrecMed) noexcept;

    /**  For column prec_fec  */
    ///Get the value of the column prec_fec, returns the default value if the column is null
    const double &getValueOfPrecFec() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<double> &getPrecFec() const noexcept;
    ///Set the value of the column prec_fec
    void setPrecFec(const double &pPrecFec) noexcept;

    /**  For column prec_exer  */
    ///Get the value of the column prec_exer, returns the default value if the column is null
    const double &getValueOfPrecExer() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<double> &getPrecExer() const noexcept;
    ///Set the value of the column prec_exer
    void setPrecExer(const double &pPrecExer) noexcept;

    /**  For column dt_datven  */
    ///Get the value of the column dt_datven, returns the default value if the column is null
    const ::trantor::Date &getValueOfDtDatven() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<::trantor::Date> &getDtDatven() const noexcept;
    ///Set the value of the column dt_datven
    void setDtDatven(const ::trantor::Date &pDtDatven) noexcept;

    /**  For column fat_cot  */
    ///Get the value of the column fat_cot, returns the default value if the column is null
    const int32_t &getValueOfFatCot() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<int32_t> &getFatCot() const noexcept;
    ///Set the value of the column fat_cot
    void setFatCot(const int32_t &pFatCot) noexcept;

    /**  For column nr_dismes  */
    ///Get the value of the column nr_dismes, returns the default value if the column is null
    const int32_t &getValueOfNrDismes() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<int32_t> &getNrDismes() const noexcept;
    ///Set the value of the column nr_dismes
    void setNrDismes(const int32_t &pNrDismes) noexcept;


    static size_t getColumnNumber() noexcept {  return 16;  }
    static const std::string &getColumnName(size_t index) noexcept(false);

    Json::Value toJson() const;
    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const;
    /// Relationship interfaces
  private:
    friend drogon::orm::Mapper<TcotBovespa>;
    friend drogon::orm::BaseBuilder<TcotBovespa, true, true>;
    friend drogon::orm::BaseBuilder<TcotBovespa, true, false>;
    friend drogon::orm::BaseBuilder<TcotBovespa, false, true>;
    friend drogon::orm::BaseBuilder<TcotBovespa, false, false>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<TcotBovespa>;
#endif
    static const std::vector<std::string> &insertColumns() noexcept;
    void outputArgs(drogon::orm::internal::SqlBinder &binder) const;
    const std::vector<std::string> updateColumns() const;
    void updateArgs(drogon::orm::internal::SqlBinder &binder) const;
    ///For mysql or sqlite3
    void updateId(const uint64_t id);
    std::shared_ptr<::trantor::Date> dtPregao_;
    std::shared_ptr<int32_t> przTermo_;
    std::shared_ptr<std::string> cdCodneg_;
    std::shared_ptr<int32_t> cdTpmerc_;
    std::shared_ptr<int32_t> cdCodbdi_;
    std::shared_ptr<std::string> cdCodisin_;
    std::shared_ptr<std::string> nmSpeci_;
    std::shared_ptr<double> precAber_;
    std::shared_ptr<double> precMax_;
    std::shared_ptr<double> precMin_;
    std::shared_ptr<double> precMed_;
    std::shared_ptr<double> precFec_;
    std::shared_ptr<double> precExer_;
    std::shared_ptr<::trantor::Date> dtDatven_;
    std::shared_ptr<int32_t> fatCot_;
    std::shared_ptr<int32_t> nrDismes_;
    struct MetaData
    {
        const std::string colName_;
        const std::string colType_;
        const std::string colDatabaseType_;
        const ssize_t colLength_;
        const bool isAutoVal_;
        const bool isPrimaryKey_;
        const bool notNull_;
    };
    static const std::vector<MetaData> metaData_;
    bool dirtyFlag_[16]={ false };
  public:
    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string sql="select * from " + tableName + " where dt_pregao = $1 and prz_termo = $2 and cd_codneg = $3";
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="delete from " + tableName + " where dt_pregao = $1 and prz_termo = $2 and cd_codneg = $3";
        return sql;
    }
    std::string sqlForInserting(bool &needSelection) const
    {
        std::string sql="insert into " + tableName + " (";
        size_t parametersCount = 0;
        needSelection = false;
        if(dirtyFlag_[0])
        {
            sql += "dt_pregao,";
            ++parametersCount;
        }
        if(dirtyFlag_[1])
        {
            sql += "prz_termo,";
            ++parametersCount;
        }
        if(dirtyFlag_[2])
        {
            sql += "cd_codneg,";
            ++parametersCount;
        }
        if(dirtyFlag_[3])
        {
            sql += "cd_tpmerc,";
            ++parametersCount;
        }
        if(dirtyFlag_[4])
        {
            sql += "cd_codbdi,";
            ++parametersCount;
        }
        if(dirtyFlag_[5])
        {
            sql += "cd_codisin,";
            ++parametersCount;
        }
        if(dirtyFlag_[6])
        {
            sql += "nm_speci,";
            ++parametersCount;
        }
        if(dirtyFlag_[7])
        {
            sql += "prec_aber,";
            ++parametersCount;
        }
        if(dirtyFlag_[8])
        {
            sql += "prec_max,";
            ++parametersCount;
        }
        if(dirtyFlag_[9])
        {
            sql += "prec_min,";
            ++parametersCount;
        }
        if(dirtyFlag_[10])
        {
            sql += "prec_med,";
            ++parametersCount;
        }
        if(dirtyFlag_[11])
        {
            sql += "prec_fec,";
            ++parametersCount;
        }
        if(dirtyFlag_[12])
        {
            sql += "prec_exer,";
            ++parametersCount;
        }
        if(dirtyFlag_[13])
        {
            sql += "dt_datven,";
            ++parametersCount;
        }
        if(dirtyFlag_[14])
        {
            sql += "fat_cot,";
            ++parametersCount;
        }
        if(dirtyFlag_[15])
        {
            sql += "nr_dismes,";
            ++parametersCount;
        }
        if(parametersCount > 0)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        int placeholder=1;
        char placeholderStr[64];
        size_t n=0;
        if(dirtyFlag_[0])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[1])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[2])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[3])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[4])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[5])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[6])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[7])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[8])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[9])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[10])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[11])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[12])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[13])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[14])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[15])
        {
            n = snprintf(placeholderStr,sizeof(placeholderStr),"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(parametersCount > 0)
        {
            sql.resize(sql.length() - 1);
        }
        if(needSelection)
        {
            sql.append(") returning *");
        }
        else
        {
            sql.append(1, ')');
        }
        LOG_TRACE << sql;
        return sql;
    }
};
} // namespace testdb
} // namespace drogon_model
